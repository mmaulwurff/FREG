" Vim syntax file
" Language: freg maps
" Maintainer: Alexander Kromm
" Latest Revision: 2014-04-17

if exists("b:current_syntax")
    finish
endif

" Keywords
syn match fregMapElementWater  '\~'
syn match fregMapElementPlain  '\.'
syn match fregMapElementHill   '+'
syn match fregMapElementForest '%'
syn match fregMapElementMount  '\^'
syn match fregMapElementNull   '#'

let b:current_syntax = "fregMap"

hi fregMapElementWater  ctermfg=Cyan    ctermbg=Blue
hi fregMapElementPlain  ctermfg=Black   ctermbg=Green
hi fregMapElementHill   ctermfg=White   ctermbg=Green
hi fregMapElementMount  ctermfg=Black   ctermbg=White
hi fregMapElementForest ctermfg=Yellow  ctermbg=Green
hi fregMapElementNull   ctermfg=Magenta ctermbg=Black
