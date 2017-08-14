/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   real.h
 * Author: simon
 *
 * Created on 14 August 2017, 17:25
 */

#ifndef REAL_H
#define REAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a real number cell representing this value and return a cons 
 * pointer to it.
 * @param value the value to wrap;
 * @return a real number cell wrapping this value.
 */
    struct cons_pointer make_real(double value);

#ifdef __cplusplus
}
#endif
#endif                          /* REAL_H */
