# sed script to help converting snippets of code from 0.0.X to 0.1.X

s?allocate_cell( *\([A-Z]*\) *)?allocate( \1, 2)?g
s?c_car(?car(?g
s?c_cdr(?cdr(?g
s?cons_pointer?pso_pointer?g
s?consspaceobject\.h?pso2\.h?
s?cons_space_object?pso2?g
s?debug_print(\([^)]*\))?debug_print(\1, 0)?g
s?frame->arg?frame->payload.stack_frame.arg?g
s?make_cons?cons?g
s?NIL?nil?g
s?nilTAG?NILTAG?g
s?&pointer2cell?pointer_to_object?g
s?stack_frame?pso4?g
s?stack\.h?pso4\.h?
s?tag.value?header.tag.bytes.value \& 0xfffff?g