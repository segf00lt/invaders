let project_root = getcwd()
let project_build = project_root . '/nob' 
let project_exe = '/invaders'
let project_run = project_root . project_exe
let project_debug = 'gf2 ' . project_root . project_exe

let &makeprg = project_build

nnoremap <F7> :call jobstart('alacritty --working-directory ' . project_root, { 'detach':v:true })<CR>
nnoremap <F8> :call chdir(project_root)<CR>
nnoremap <F9> :wa<CR>:make<CR>
nnoremap <F10> :call jobstart(project_run, { 'detach':v:true })<CR>
nnoremap <F11> :call jobstart(project_debug, { 'detach':v:true })<CR>

