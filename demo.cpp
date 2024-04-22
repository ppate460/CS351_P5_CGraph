
#include <iostream>
#include "cpath.h"
#include "pqueue.h"

using namespace std;

int main(){
  cpath g;

  g.read_file("edges.txt");

  cout<<"PATHS FROM 0 to 6: "<<endl;
  g.cpaths(0, 6, 10);
  cout<<endl;

  g.display_edges_from_each_vertex();

  return 0;
}

