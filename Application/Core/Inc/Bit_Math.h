/*
 * Bit_Math.h
 *
 *  Created on: Aug 24, 2023
 *      Author: Scorpio
 */

#ifndef INC_BIT_MATH_H_
#define INC_BIT_MATH_H_

#define SET_BIT(VAR,BIT)          VAR |=  (1 << (BIT))
#define CLR_BIT(VAR,BIT)          VAR &= ~(1 << (BIT))
#define GET_BIT(VAR,BIT)          (((VAR) >> BIT) & 1  )
#define TOG_BIT(VAR,BIT)          VAR ^=  (1 << (BIT))

#endif /* INC_BIT_MATH_H_ */
