// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of 
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "ligra.h"

struct BFS_F {
  uintE* Parents;
  BFS_F(uintE* _Parents) : Parents(_Parents) {}
  inline bool update (uintE s, uintE d) { //Update
    if(Parents[d] == UINT_E_MAX) { Parents[d] = s; return 1; }
    else return 0;
  }
  inline bool updateAtomic (uintE s, uintE d){ //atomic version of Update
    return (CAS(&Parents[d],UINT_E_MAX,s));
  }
  //cond function checks if vertex has been visited yet
  inline bool cond (uintE d) { return (Parents[d] == UINT_E_MAX); } 
};

template <class vertex>
void Compute(graph<vertex>& GA, commandLine P) {
  long start = P.getOptionLongValue("-r",0);
  long n = GA.n;
  //creates Parents array, initialized to all -1, except for start
  uintE* Parents = newA(uintE,n);
  parallel_for(long i=0;i<n;i++) Parents[i] = UINT_E_MAX;
  Parents[start] = start;

  ////////////////////////////////////////////////////////////////
  // By Johnpzh
  long *dists = newA(long, n);
  parallel_for(long i=0;i<n;i++) dists[i] = UINT_E_MAX;
  dists[start] = 0;
  startTime(); //
  // End by Johnpzh
  ////////////////////////////////////////////////////////////////

  vertexSubset Frontier(n,start); //creates initial frontier
  while(!Frontier.isEmpty()){ //loop until frontier is empty
    vertexSubset output = edgeMap(GA, Frontier, BFS_F(Parents));    
    Frontier.del();
    Frontier = output; //set new frontier

	////////////////////////////////////////////////////////////////
	// By Johnpzh
		if (Frontier.d) {
			{parallel_for (long i = 0; i < Frontier.n; ++i) {
				if (Frontier.d[i]) {
				    uintE s_i = Parents[i];
				    dists[i] = dists[s_i] + 1;
				}
			}}
		} else if (Frontier.s) {
			parallel_for (long i = 0; i < Frontier.m; ++i) {
				uintE v_i = Frontier.s[i];
				uintE s_i = Parents[v_i];
				dists[v_i] = dists[s_i] + 1;
			}
		} 
	// End by Johnpzh
	////////////////////////////////////////////////////////////////
  } 

  // By Johnpzh
  nextTime(""); //
  FILE *fout = fopen("results.txt", "w");
  for(long i = 0; i < n; i ++) {
	  //fprintf(fout, "dists[%ld]: %ld\n", i, dists[i]);
	  //fprintf(fout, "%d) cost:%ld\n", i, dists[i] != UINT_E_MAX ? dists[i] : -1);
	  fprintf(fout, "%d: %ld\n", i, dists[i]);
  }
  fclose(fout);
  free(dists);
  // End by Johnpzh

  Frontier.del();
  free(Parents); 
}
