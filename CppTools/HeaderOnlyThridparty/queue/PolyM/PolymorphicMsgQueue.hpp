/******* 
 * 
 * *   https://github.com/khuttun/PolyM
 * 
 *******/

#pragma once

#ifndef __MSG_QUEUE_HPP__
#define __MSG_QUEUE_HPP__

#include <atomic>
#include <memory>
#include <utility>

#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>

namespace PolyM {

/** Type for Msg unique identifiers */
// using MsgUID = unsigned long long;

/**
 * Msg represents a simple message that doesn't have any payload data.
 * Msg ID identifies the type of the message. Msg ID can be queried with
 * getMsgId().
 */
class Msg {
public:
  /**
   * Construct a Msg.
   *
   * @param msgId Msg ID of this Msg.
   */
  Msg(int msgId);

  virtual ~Msg() = default;
  Msg(const Msg &) = delete;
  Msg &operator=(const Msg &) = delete;

  /** "Virtual move constructor" */
  virtual std::unique_ptr<Msg> move();

  /**
   * Get Msg ID.
   * Msg ID identifies message type.
   * Multiple Msg instances can have the same Msg ID.
   */
  int getMsgId() const;

  /**
   * Get Msg UID.
   * Msg UID is the unique ID associated with this message.
   * All Msg instances have a unique Msg UID.
   */
  unsigned long long getUniqueId() const;

protected:
  Msg(Msg &&) = default;
  Msg &operator=(Msg &&) = default;

private:
  int msgId_;
  unsigned long long uniqueId_;
};

/**
 * DataMsg<PayloadType> is a Msg with payload of type PayloadType.
 * Payload is constructed when DataMsg is created and the DataMsg instance owns
 * the payload data.
 */
template <typename PayloadType> class DataMsg : public Msg {
public:
  /**
   * Construct DataMsg
   * @param msgId Msg ID
   * @param args Arguments for PayloadType ctor
   */
  template <typename... Args>
  DataMsg(int msgId, Args &&... args)
      : Msg(msgId), pl_(new PayloadType(std::forward<Args>(args)...)) {}

  virtual ~DataMsg() = default;
  DataMsg(const DataMsg &) = delete;
  DataMsg &operator=(const DataMsg &) = delete;

  /** "Virtual move constructor" */
  virtual std::unique_ptr<Msg> move() override {
    return std::unique_ptr<Msg>(new DataMsg<PayloadType>(std::move(*this)));
  }

  /** Get the payload data */
  PayloadType &getPayload() const { return *pl_; }

protected:
  DataMsg(DataMsg &&) = default;
  DataMsg &operator=(DataMsg &&) = default;

private:
  std::unique_ptr<PayloadType> pl_;
};


namespace {


unsigned long long generateUniqueId() {
  static std::atomic<unsigned long long> i(0);
  return ++i;
}

} // namespace

Msg::Msg(int msgId) : msgId_(msgId), uniqueId_(generateUniqueId()) {}

std::unique_ptr<Msg> Msg::move() {
  return std::unique_ptr<Msg>(new Msg(std::move(*this)));
}

int Msg::getMsgId() const { return msgId_; }

unsigned long long Msg::getUniqueId() const { return uniqueId_; }

} // namespace PolyM

namespace PolyM {

/**
 * Queue is a thread-safe message queue.
 * It supports one-way messaging and request-response pattern.
 */
class Queue {
public:
  Queue();

  ~Queue();

  /**
   * Put Msg to the end of the queue.
   *
   * @param msg Msg to put to the queue.
   */
  void put(Msg &&msg);

  /**
   * Get message from the head of the queue.
   * Blocks until at least one message is available in the queue, or until
   * timeout happens. If get() returns due to timeout, returns a nullptr.
   *
   * @param timeoutMillis How many ms to wait for message until timeout happens.
   *                      0 = wait indefinitely.
   */
  std::unique_ptr<Msg> get(int timeoutMillis = 0);

  /**
   * Get message from the head of the queue.
   * Returns an empty pointer if no message is available.
   */
  std::unique_ptr<Msg> tryGet();

  /**
   * Make a request.
   * Call will block until response is given with respondTo().
   * If request() returns due to timeout, returns a nullptr.
   *
   * @param msg Request message. Is put to the queue so it can be retrieved from
   * it with get().
   * @param timeoutMillis How many ms to wait for response until timeout
   * happens. 0 = wait indefinitely.
   */
  std::unique_ptr<Msg> request(Msg &&msg, int timeoutMillis = 0);

  /**
   * Respond to a request previously made with request().
   * If the requestID has been found, return true.
   *
   * @param reqUid Msg UID of the request message.
   * @param responseMsg Response message. The requester will receive it as the
   * return value of request().
   */
  bool respondTo(unsigned long long reqUid, Msg &&responseMsg);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};


class Queue::Impl {
  struct Request {
    Request(){};

    std::unique_ptr<Msg> response;
    std::condition_variable condVar;
  };

public:
  Impl()
      : queue_(), queueMutex_(), queueCond_(), responseMap_(),
        responseMapMutex_() {}

  void put(Msg &&msg) {
    {
      std::lock_guard<std::mutex> lock(queueMutex_);
      queue_.push(msg.move());
    }

    queueCond_.notify_one();
  }

  std::unique_ptr<Msg> get(int timeoutMillis) {
    std::unique_lock<std::mutex> lock(queueMutex_);

    if (timeoutMillis <= 0)
      queueCond_.wait(lock, [this] { return !queue_.empty(); });
    else {
      // wait_for returns false if the return is due to timeout
      auto timeoutOccured =
          !queueCond_.wait_for(lock, std::chrono::milliseconds(timeoutMillis),
                               [this] { return !queue_.empty(); });

      if (timeoutOccured)
        return nullptr;
    }

    auto msg = queue_.front()->move();
    queue_.pop();
    return msg;
  }

  std::unique_ptr<Msg> tryGet() {
    std::unique_lock<std::mutex> lock(queueMutex_);
    if (!queue_.empty()) {
      auto msg = queue_.front()->move();
      queue_.pop();
      return msg;
    } else {
      return {nullptr};
    }
  }

  std::unique_ptr<Msg> request(Msg &&msg, int timeoutMillis) {
    Request req;

    // emplace the request in the map
    std::unique_lock<std::mutex> lock(responseMapMutex_);
    auto it =
        responseMap_.emplace(std::make_pair(msg.getUniqueId(), &req)).first;

    put(std::move(msg));

    if (timeoutMillis <= 0)
      req.condVar.wait(lock, [&req] { return req.response.get(); });
    else {
      // wait_for returns false if the return is due to timeout
      auto timeoutOccured =
          !req.condVar.wait_for(lock, std::chrono::milliseconds(timeoutMillis),
                                [&req] { return req.response.get(); });

      if (timeoutOccured) {
        responseMap_.erase(it);
        return nullptr;
      }
    }

    auto response = std::move(it->second->response);
    responseMap_.erase(it);

    return response;
  }

  bool respondTo(unsigned long long reqUid, Msg &&responseMsg) {
    std::unique_lock<std::mutex> lock(responseMapMutex_);
    auto it = responseMap_.find(reqUid);
    if (it == responseMap_.end())
      return false;

    it->second->response = responseMsg.move();
    it->second->condVar.notify_one();
    return true;
  }

private:
  // Queue for the Msgs
  std::queue<std::unique_ptr<Msg>> queue_;

  // Mutex to protect access to the queue
  std::mutex queueMutex_;

  // Condition variable to wait for when getting Msgs from the queue
  std::condition_variable queueCond_;

  // Map to keep track of which request IDs are associated with which request
  // Msgs
  std::map<unsigned long long, Request *> responseMap_;

  // Mutex to protect access to response map
  std::mutex responseMapMutex_;
};

Queue::Queue() : impl_(new Impl) {}

Queue::~Queue() {}

void Queue::put(Msg &&msg) { impl_->put(std::move(msg)); }

std::unique_ptr<Msg> Queue::get(int timeoutMillis) {
  return impl_->get(timeoutMillis);
}

std::unique_ptr<Msg> Queue::tryGet() { return impl_->tryGet(); }

std::unique_ptr<Msg> Queue::request(Msg &&msg, int timeoutMillis) {
  return impl_->request(std::move(msg), timeoutMillis);
}

bool Queue::respondTo(unsigned long long reqUid, Msg &&responseMsg) {
  return impl_->respondTo(reqUid, std::move(responseMsg));
}

} // namespace PolyM

#endif   //__MSG_QUEUE_HPP__