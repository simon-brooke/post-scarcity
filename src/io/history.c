/*
 * history.c
 * 
 * Maintain, and recall, a history of things which have been read from standard
 * input. Necessarily the history must be stored on the user session, and not be
 * global.
 *
 * I *think* history will be maintained as a list of forms, not of strings, so
 * only forms which have successfully been read can be recalled, and forms which
 * have not been completed when the history function is invoked will be lost. 
 *
 * (c) 2025 Simon Brooke <simon@journeyman.cc>
 * Licensed under GPL version 2.0, or, at your option, any later version.
 */