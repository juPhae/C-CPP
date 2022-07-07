/*******************************************************************************
 *FileName: base64.hpp
 *version:
 *Author: phae
 *Date: 2022-05-17 16:16:45
 *Description:
 *LastEditors: phae
 *LastEditTime: 2022-07-07 11:30:02
 ********************************************************************************/
#ifndef __BASE64_HPP__
#define __BASE64_HPP__

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#define IMG_JPEG "data:image/jpeg;base64,"

class CBase64 {
 public:
  CBase64() = default;
  ~CBase64() = default;

  /*编码
  DataByte
  [in]输入的数据长度,以字节为单位
  */
  std::string Encode(const char* Data, int DataByte);

  /*解码
  DataByte
  [in]输入的数据长度,以字节为单位
  OutByte
  [out]输出的数据长度,以字节为单位,请不要通过返回值计算
  输出数据的长度
  */
  std::string Decode(const char* Data, int DataByte, int& OutByte);

  /*******************************************************************************
    *name:  Decode
    *descripttion:  重载

    *param  encoded_string

   ********************************************************************************/
  std::string Decode(std::string const& encoded_string);

  /*******************************************************************************
   *name: Base64ToPicture
   *descripttion: file_output_path要包含图片名。 例如 /home/testbak.jpeg
   ********************************************************************************/

  void Base64ToPicture(std::string img, std::string file_output_path);

  /*******************************************************************************
   *name: PictureToBase64
   *descripttion: picture_path要包含图片名。 例如 /home/test.jpeg
   ********************************************************************************/
  std::string PictureToBase64(std::string picture_path);

 private:
  static inline bool is_base64(const char c);
  static const std::string base64_chars;
};

inline bool CBase64::is_base64(const char c) { return (isalnum(c) || (c == '+') || (c == '/')); }

const std::string CBase64::base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string CBase64::Encode(const char* Data, int DataByte) {
  //编码表
  const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  //返回值
  std::string strEncode;
  unsigned char Tmp[4] = {0};
  int LineLength = 0;
  for (int i = 0; i < (int)(DataByte / 3); i++) {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    Tmp[3] = *Data++;
    strEncode += EncodeTable[Tmp[1] >> 2];
    strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
    strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
    strEncode += EncodeTable[Tmp[3] & 0x3F];
    if (LineLength += 4, LineLength == 76) {
      strEncode += "\r\n";
      LineLength = 0;
    }
  }
  //对剩余数据进行编码
  int Mod = DataByte % 3;
  if (Mod == 1) {
    Tmp[1] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
    strEncode += "==";
  } else if (Mod == 2) {
    Tmp[1] = *Data++;
    Tmp[2] = *Data++;
    strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
    strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
    strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
    strEncode += "=";
  }

  return strEncode;
}

std::string CBase64::Decode(const char* Data, int DataByte, int& OutByte) {
  //解码表
  const char DecodeTable[] = {
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
      62,  // '+'
      0,  0,  0,
      63,                                      // '/'
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  // '0'-'9'
      0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  // 'A'-'Z'
      0,  0,  0,  0,  0,  0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
      42, 43, 44, 45, 46, 47, 48, 49, 50, 51,  // 'a'-'z'
  };
  //返回值
  std::string strDecode;
  int nValue;
  int i = 0;
  while (i < DataByte) {
    if (*Data != '\r' && *Data != '\n') {
      nValue = DecodeTable[*Data++] << 18;
      nValue += DecodeTable[*Data++] << 12;
      strDecode += (nValue & 0x00FF0000) >> 16;
      OutByte++;
      if (*Data != '=') {
        nValue += DecodeTable[*Data++] << 6;
        strDecode += (nValue & 0x0000FF00) >> 8;
        OutByte++;
        if (*Data != '=') {
          nValue += DecodeTable[*Data++];
          strDecode += nValue & 0x000000FF;
          OutByte++;
        }
      }
      i += 4;
    } else  // 回车换行,跳过
    {
      Data++;
      i++;
    }
  }
  return strDecode;
}

std::string CBase64::Decode(std::string const& encoded_string) {
  int in_len = (int)encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_];
    in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++) char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++) ret += char_array_3[i];
      i = 0;
    }
  }
  if (i) {
    for (j = i; j < 4; j++) char_array_4[j] = 0;

    for (j = 0; j < 4; j++) char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

std::string CBase64::PictureToBase64(std::string picture_path) {
  std::ifstream fin(picture_path, std::ios::binary);
  fin.seekg(0, std::ios::end);
  int iSize = fin.tellg();
  char* szBuf = new (std::nothrow) char[iSize];

  fin.seekg(0, std::ios::beg);
  fin.read(szBuf, sizeof(char) * iSize);

  fin.close();
  std::string img = IMG_JPEG + Encode(szBuf, iSize);

  return img;
}

void CBase64::Base64ToPicture(std::string img, std::string file_output_path) {
  if (img.find(IMG_JPEG) != std::string::npos) {
    img = img.replace(img.find(IMG_JPEG), sizeof(IMG_JPEG), "");  // 去处头部的 “data:image/jpeg;base64,”
  }
  std::string img_data = Decode(img);
  std::ofstream fout(file_output_path, std::ios::binary);
  fout.write(img_data.c_str(), img_data.size());
  fout.close();
}

#endif  //_BASE64_H_
