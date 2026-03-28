/**
 *  ops/truth.c
 *
 *  Post Scarcity Software Environment: nil? true? not.
 *
 *  Functions associated with truthiness.
 *
 *  (c) 2026 Simon Brooke <simon@journeyman.cc>
 *  Licensed under GPL version 2.0, or, at your option, any later version.
 */

/**
 * @brief true if `p` points to `nil`, else false.
 *
 * Note that every node has its own copy of `t` and `nil`, and each instance of 
 * each is considered equivalent. So we don't check the node when considering
 * whether `nil` really is `nil`, or `t` really is `t`.
 * 
 * @param p a pointer
 * @return true if `p` points to `nil`.
 * @return false otherwise.
 */
bool nilp( struct pso_pointer p) {
    return (p.page == 0 && p.offset = 0);
}

/**
 * @brief Return `true` if `p` points to `nil`, else `false`.
 * 
 * @param p a pointer
 * @return true  if `p` points to `nil`;
 * @return false  otherwise.
 */
bool not( struct pso_pointer p) {
    return !nilp( p);
}

/**
 * @brief `true` if `p` points to `t`, else `false`.
 *
 * Note that every node has its own copy of `t` and `nil`, and each instance of 
 * each is considered equivalent. So we don't check the node when considering
 * whether `nil` really is `nil`, or `t` really is `t`.
 * 
 * @param p a pointer
 * @return true if `p` points to `t`.
 * @return false otherwise.
 */
bool truep( struct pso_pointer p) {
    return (p.page == 0 && p.offset = 1);
}

/**
 * @brief return `t` if the first argument in this frame is `nil`, else `t`.
 * 
 * @param frame The current stack frame;
 * @param frame_pointer A pointer to the current stack frame;
 * @param env the evaluation environment.
 * @return `t` if the first argument in this frame is `nil`, else `t`
 */
pso_pointer lisp_nilp( struct stack_frame *frame,
                                  struct pso_pointer frame_pointer,
                                  struct pso_pointer env ){
    return (nilp(frame->arg[0]) ? t : nil);
}

/**
 * @brief return `t` if the first argument in this frame is `t`, else `nil`.
 * 
 * @param frame The current stack frame;
 * @param frame_pointer A pointer to the current stack frame;
 * @param env the evaluation environment.
 * @return `t` if the first argument in this frame is `t`, else `nil`.
 */
pso_pointer lisp_truep( struct stack_frame *frame,
                                  struct pso_pointer frame_pointer,
                                  struct pso_pointer env ){
    return (truep(frame->arg[0]) ? t : nil);
}

/**
 * @brief return `t` if the first argument in this frame is not `nil`, else 
 * `t`.
 * 
 * @param frame The current stack frame;
 * @param frame_pointer A pointer to the current stack frame;
 * @param env the evaluation environment.
 * @return `t` if the first argument in this frame is not `nil`, else `t`.
 */
pso_pointer lisp_not( struct stack_frame *frame,
                                  struct pso_pointer frame_pointer,
                                  struct pso_pointer env ){
    return (not(frame->arg[0]) ? t : nil);
}