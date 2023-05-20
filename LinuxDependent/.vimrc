"一个tab显示出来是多少个空格的长度
set tabstop=2
"在编辑模式的时候按退格键的时候退回缩进的长度
set softtabstop=2
"每一级缩进的长度,当设置成 expandtab 时缩进用空格来表示noexpandtab
"则是用制表符表示一个缩进
set shiftwidth=2
set noexpandtab
"显示行号
set nu
"自动缩进
set autoindent
"为C程序提供自动缩进
set smartindent
"特别针对C语言自动缩进
set cindent
"打开语法高亮
syntax on
"允许使用鼠标点击定位
set mouse=a
"允许区域选择
set selection=exclusive
set selectmode=mouse,key
"高亮被搜索的句子
set hlsearch
"继承前一行的缩进方式，特别适用于多行注释
set autoindent
"允许使用退格键，或set backspace=2
set backspace=eol,start,indent
set whichwrap+=<,>,h,l
"在被分割的窗口间显示空白，便于阅读
set fillchars=vert:\ ,stl:\ ,stlnc:\
"设定默认解码
set fenc=utf-8
set fencs=utf-8,usc-bom,euc-jp,gb18030,gbk,gb2312,cp936
"设定字体
"set guifont=Courier_New:h11:cANSI
"set guifontwide=新宋体:h11:cGB2312
"设定编码
set enc=utf-8
set fileencodings=ucs-bom,utf-8,chinese
"set langmenu=zh_CN.UTF-8
"language message zh_CN.UTF-8
source $VIMRUNTIME/delmenu.vim
source $VIMRUNTIME/menu.vim
"允许使用插件
filetype plugin on
filetype plugin indent on
"自动补全
filetype plugin indent on
set completeopt=longest,menu
"自动补全命令时候使用菜单式匹配列表
set wildmenu
autocmd FileType ruby,eruby set omnifunc=rubycomplete#Complete
autocmd FileType python set omnifunc=pythoncomplete#Complete
autocmd FileType javascript set omnifunc=javascriptcomplete#CompleteJS
autocmd FileType html set omnifunc=htmlcomplete#CompleteTags
autocmd FileType css set omnifunc=csscomplete#CompleteCSS
autocmd FileType xml set omnifunc=xmlcomplete#CompleteTags
autocmd FileType java set omnifunc=javacomplete#Complet
"设置括号自动补全
inoremap ' ''<ESC>i
inoremap " ""<ESC>i
inoremap ( ()<ESC>i
inoremap [ []<ESC>i
inoremap < <><ESC>i
inoremap { {<CR>}<ESC>O
"设置跳出自动补全的括号
func SkipPair()
	if getline('.')[col('.') - 1] == ')' || getline('.')[col('.') - 1] == ']' || getline('.')[col('.') - 1] == '"' || getline('.')[col('.') - 1] == "'" || getline('.')[col('.') - 1] == '}'
		return "\<ESC>la"
	else
		return "\t"
	endif
endfunc
"将tab键绑定为跳出括号
inoremap <TAB> <c-r>=SkipPair()<CR>
"假设你想查找某个字符串在项目中出现的位置，你可以把光标放在这个字符串上
nmap  <F6> :vimgrep /<C-R>=expand("<cword>")<cr>/ **/*.c **/*.h<cr><C-o>:cw<cr>
"C，C++ 按F7编译运行
map <F7> :call CompileRunGcc()<CR>
func! CompileRunGcc()
	exec "w"
	if &filetype == 'c'
		exec "!gcc % -o %<"
		exec "! ./%<"
	elseif &filetype == 'cpp'
		exec "!g++ % -o %<"
		exec "! ./%<"
	elseif &filetype == 'sh'
		:!./%
	elseif &filetype == 'py'
		exec "!python %"
		exec "!python %<"
	endif
endfunc
"C,C++的调试
map <F8> :call Rungdb()<CR>
func! Rungdb()
	exec "w"
	exec "!gcc % -g -o %<"
	exec "!gdb ./%<"
endfunc
" 新建文件标题: {{{1
""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""
"新建.c,.h,.sh,.java文件，自动插入文件头
autocmd BufNewFile *.cpp,*.[ch],*.sh,*.java exec ":call SetTitle()"
"新建文件后，自动定位到文件末尾
autocmd BufNewFile * normal G
"定义函数SetTitle，自动插入文件头
func SetTitle() 
	"如果文件类型为.sh文件
	if &filetype == 'sh' 
		call setline(1,"\#########################################################################") 
		call append(line("."), "\# File Name: ".expand("%")) 
	  call append(line(".")+1, "\# Author: Phae") 
		call append(line(".")+2, "\# mail: 9659900@gmail.com") 
		call append(line(".")+3, "\# Created Time: ".strftime("%c")) 
		call append(line(".")+4, "\#########################################################################") 
		call append(line(".")+5, "\#!/bin/bash") 
		call append(line(".")+6, "") 
	else 
		call setline(1, "/*************************************************************************") 
		call append(line("."), "    > File Name: ".expand("%")) 
		call append(line(".")+1, "    > Author: Phae") 
		call append(line(".")+2, "    > Mail: 9659900@gmail.com ") 
		call append(line(".")+3, "    > Created Time: ".strftime("%c")) 
		call append(line(".")+4, " ************************************************************************/") 
		call append(line(".")+5, "")
	endif
	"if &filetype == 'cpp'
	"	call append(line(".")+6, "#include<bits/stdc++.h>")
	"	call append(line(".")+7, "#include <unistd.h>")
	"	call append(line(".")+8, "#include <sys/types.h>")
	"	call append(line(".")+9, "#include <sys/time.h>")
	"	call append(line(".")+10, "")
	"endif
	if &filetype == 'c'
		call append(line(".")+6, "#include<stdio.h>")
		call append(line(".")+7, "")
	endif
	"	if &filetype == 'java'
	"		call append(line(".")+6,"public class ".expand("%"))
	"		call append(line(".")+7,"")
	" endif
endfunc

