/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   repl.h
 * Author: simon
 *
 * Created on 14 August 2017, 14:40
 */

#ifndef REPL_H
#define REPL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The read/eval/print loop
 * @param in_stream the stream to read from;
 * @param out_stream the stream to write to;
 * @param err_stream the stream to send errors to;
 * @param show_prompt true if prompts should be shown.
 */
    void repl(FILE * in_stream, FILE * out_stream,
              FILE * error_stream, bool show_prompt);

#ifdef __cplusplus
}
#endif
#endif                          /* REPL_H */
