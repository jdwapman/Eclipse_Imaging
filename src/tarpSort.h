/*
 * tarpSort.h
 *
 *  Created on: Jan 3, 2018
 *      Author: jwapman
 */

#ifndef TARPSORT_H_
#define TARPSORT_H_

#include <tuple>
#include <vector>

bool sortAreas(const tuple<double, unsigned int> &lhs,
               const tuple<double, unsigned int> &rhs);

bool sortVertices(const tuple<unsigned int, unsigned int> &lhs,
                  const tuple<unsigned int, unsigned int> &rhs);

#endif /* TARPSORT_H_ */
