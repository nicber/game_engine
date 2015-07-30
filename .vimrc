set nocompatible " be iMproved, required
filetype off " required
" set the runtime path to include Vundle and initialize
set rtp+=~/.vim/bundle/vundle/
call vundle#rc() " alternatively, pass a path where Vundle should install plugins
             "let path = '~/some/path/here'
             "call vundle#rc(path)
             " let Vundle manage Vundle, required

Plugin 'Valloric/YouCompleteMe'
Plugin 'altercation/vim-colors-solarized'
Plugin 'bling/vim-airline'
Plugin 'bling/vim-bufferline'
Plugin 'bronson/vim-trailing-whitespace'
Plugin 'gmarik/Vundle.vim'
Plugin 'godlygeek/tabular'
Plugin 'justinmk/vim-sneak'
Plugin 'kien/ctrlp.vim'
Plugin 'qpkorr/vim-bufkill'
Plugin 'scrooloose/nerdtree.git'
Plugin 'tpope/vim-fugitive'

filetype plugin indent on
syntax enable

let g:airline_powerline_fonts = 1
let g:ycm_autoclose_preview_window_after_completion = 1

set background=dark
colorscheme solarized

if (exists('+colorcolumn'))
    set colorcolumn=80
    highlight ColorColumn ctermbg=0
endif

command Format :w | silent execute "!clang-format -style=file -i %:p" | :redraw! | :e

if !has('gui_running')
  set ttimeoutlen=10
  augroup FastEscape
    autocmd!
    au InsertEnter * set timeoutlen=0
    au InsertLeave * set timeoutlen=1000
  augroup END
endif

set relativenumber

set ghr=0

set mouse=a

set tabstop=2
set expandtab
set shiftwidth=2
set softtabstop=2

set laststatus=2 " Always display the statusline in all windows
set noshowmode " Hide the default mode text (e.g. -- INSERT -- below the statusline)

set wildmenu
set wildmode=list:longest,full

"https://gist.github.com/MaienM/1258015
inoremap <silent> = =<Esc>:call <SID>ealign()<CR>a
function! s:ealign()
  let p = '^.*=\s.*$'
  if exists(':Tabularize') && getline('.') =~# '^.*=' && (getline(line('.')-1) =~# p || getline(line('.')+1) =~# p)
    let column = strlen(substitute(getline('.')[0:col('.')],'[^=]','','g'))
    let position = strlen(matchstr(getline('.')[0:col('.')],'.*=\s*\zs.*'))
    Tabularize/=/l1
    normal! 0
    call search(repeat('[^=]*=',column).'\s\{-\}'.repeat('.',position),'ce',line('.'))
  endif
endfunction

