/*
 * tarpSort.cpp
 *
 *  Created on: Jan 3, 2018
 *      Author: jwapman
 */

#include <tuple>
#include <vector>

using namespace std;

// Sort by area largest to smallest
bool sortAreas(const tuple<double, unsigned int> &lhs,
               const tuple<double, unsigned int> &rhs)
{
  return (get<0>(lhs) > get<0>(rhs));
}

bool sortVertices(const tuple<unsigned int, unsigned int> &lhs,
                  const tuple<unsigned int, unsigned int> &rhs)
{
  return (get<0>(lhs) > get<0>(rhs));
}
