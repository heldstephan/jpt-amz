#ifndef _SEGMENT_H
#define _SEGMENT_H

/*
 * This header specifies the interface for accessing and manipulating a
 * tour. 
 *
 * The tour representation support the following primitive operations:
 *
 *     (1) find the predecessor of a node in the tour with respect 
 *         to a chosen orientation (PRED);
 *
 *     (2) find the successor of a node in the tour with respect to 
 *         a chosen orientation (SUC); 
 *
 *     (3) determine whether a given node is between two other nodes 
 *         in the tour with respect to a chosen orientation (BETWEEN);
 *
 *     (4) make a 2-opt move (FLIP).
 *	
 */

#define PRED(a) (Reversed ? (a)->Suc : (a)->Pred)
#define SUC(a) (Reversed ? (a)->Pred : (a)->Suc)
#define BETWEEN(a, b, c) Between(a, b, c)
#define FLIP(a, b, c, d) Flip(a, b, c)

#define Swap1(a1,a2,a3)\
        FLIP(a1,a2,a3,0)
#define Swap2(a1,a2,a3, b1,b2,b3)\
        (Swap1(a1,a2,a3), Swap1(b1,b2,b3))
#define Swap3(a1,a2,a3, b1,b2,b3, c1,c2,c3)\
        (Swap2(a1,a2,a3, b1,b2,b3), Swap1(c1,c2,c3))

#endif
