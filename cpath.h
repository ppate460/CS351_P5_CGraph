#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <fstream>

#include <queue>

using namespace std;

#define UNDISCOVERED 'u'
#define DISCOVERED   'd'
#define ACTIVE       'a'
#define FINISHED     'f'
#define MAX 9999999999.999

class cpath {

    private:

    struct vertex_label {
      double cost;
      double time;
      int pred;
      int id;
      char state;
      int npaths;
      
      vertex_label( double _cost=0.0, double _time=0.0, int _pred=-1, char _state=UNDISCOVERED,
          int _npaths=0) 
        : cost { _cost }, time { _time }, pred { _pred }, state { _state}, npaths { 0 }
      { }
    };

    //Edge of a vertex 
    struct edge {
      int vertex_id;
      double cost;
      double time; //Added Augmentation
      edge ( int vtx_id=0, double _weight=1.0, double _time = 1.0) 
        : vertex_id { vtx_id}, cost { _weight}, time{ _time } 
      { }
    };

    struct vertex {
      int id;
      vector<edge> outgoing;
      vector<edge> incoming;
      string name;
      vector<vector<vertex_label>> candidate_paths; // candidate paths?? 
      vertex ( int _id=0, string _name="") 
        : id { _id }, name { _name } 
      { }
    };

    unordered_map<string, int> _name2id; // _name2id is an unordered_map (hash map) which lets us easily retrieve the vertex ID associated with a given vertex name (it maps from strings to integers)

    vector<vertex> vertices;

    unordered_set<string> edges;

    using Path = vector<vertex_label>;
    Path labels;


    public:

        cpath() {}

        ~cpath() {}

        int add_vertex(const string &name) {
            int id = vertices.size();
            vertices.push_back(vertex(id, name));
            vertex_label label;
            label.id = id;
            labels.push_back(label);
            _name2id[name] = id;
            return id;
        }

        //returns vertex name (a string) associated with given id
        string  id2name(int id) {
            if(id<0 || id>=vertices.size())
                return "$NONE$";
            return vertices[id].name;
        }
    
        //returns integer vertex id of given vertex name
        int name2id(const string &vtx_name) {
            if(_name2id.count(vtx_name)==0)
                return -1;
            return _name2id[vtx_name];
        }

        static string edge_string(const string &src, const string &dest) {
            return src + " " + dest;
        }

        bool add_edge(const string &src, const string &dest, double weight=1.0, double time=1.0) {

            int s_id, d_id;

            string estring = edge_string(src, dest);

            if(edges.count(estring)==1) {
                std::cerr << "warning: duplicate edge '"
                << estring << "'\n";
                return false;
            }

            edges.insert(estring);

            // get id for source vertex
            if(_name2id.count(src)==0) 
                s_id = add_vertex(src);
            else
                s_id = _name2id[src];

            // get id for destination vertex
            if(_name2id.count(dest)==0) 
                d_id = add_vertex(dest);
            else
                d_id = _name2id[dest];

            vertices[s_id].outgoing.push_back(edge(d_id, weight, time));
            vertices[d_id].incoming.push_back(edge(s_id, weight, time));

            return true;
        }

        bool add_edge(const string &str) {
            std::stringstream ss(str);
            string src, dest, cost, time, junk;
            double weight;
            double duration;

            if(!(ss >> src))
                return false;
            if(!(ss >> dest))
                return false;
            if(!(ss >> cost)){
                // only two tokens: use default weight
                weight = 1.0;
            }
            if(!(ss >> time)){
                duration = 1.0;
            }
            else {
                if(!(std::stringstream(cost) >> weight)){
                // couldn't parse weight
                return false;
                }

                // TODO: add code to parse 2nd weight if it exists here
                //   for cpaths
                if(!(std::stringstream(time) >> duration)){ //Added parsing of 2nd weight
                // couldn't parse duration
                return false;
                }


                if(ss >> junk){
                // extra token?  format error
                return false;
                }
            }

            add_edge(src, dest, weight, duration);

            return true;
        }

        void display_edges_from_each_vertex(){
            if(vertices.size()==0){
                cout<<"GRAPH IS EMPTY"<<endl;
                return;
            }

            for(vertex v : vertices){
                cout<<"- From vertex "<<v.name<<" with ID "<<v.id<<" you can go to:"<<endl;
                for(edge e : v.outgoing){
                    cout<<"    Vertex Name: "<<id2name(e.vertex_id)<<" Vertex ID: "<<e.vertex_id<< " Cost: "<<e.cost<<" Time: "<<e.time<<endl;
                }
            }
        }

        //Function that reads a file name and adds the edges based on the file
        bool read_file(const string &fname) {
            std::ifstream file;
            string line;

            file.open(fname, std::ios::in);
            if(!file.is_open())
                return false;
            
            while(getline(file, line)) {
                // skip blank lines
                if(line.length() > 0) {
                if(!add_edge(line)) {
                    std::cerr << "warning: skipped input line '" 
                    << line << "' (ill-formatted)\n";
                }
                }
            }
            file.close();
            return true;
        }

        //Retrieves number of vertices and edges in the graph
        int num_nodes() {
            return vertices.size();
        }
        int num_edges() {
            return edges.size();
        }

        void init_report(std::vector<vertex_label> & report) {
            int u;

            report.clear();
            for(u=0; u<vertices.size(); u++) {
                report.push_back(vertex_label(-1, -1, -1, UNDISCOVERED));
            }
        }

        class Compare {
            public:
                bool operator()(Path path1, Path path2) {
                    double totalCost1 = path1.back().cost;
                    double totalCost2 = path2.back().cost;

                    return totalCost1 > totalCost2; // Sort in descending order of total cost
                }
        };

        //Find all paths from src to dest whose total budget is equal to or less than the budget parameter
        void cpaths(int src, int dest, double budget) {
            priority_queue<Path, vector<Path>, Compare> q;

            Path new_labels = labels;

            Path path;
            int path_index = 0;
            find_cpaths(src, dest, new_labels, path_index, path, q);

            double best_time = MAX;
            vector<Path> budget_paths;
            Path best_path;
            
            cout<<"ALL PATHS: "<<endl;
            while (!q.empty())
            {
                Path p = q.top();
                double cost;
                double time;
                for(auto v : p){
                    cout<<id2name(v.id)<<" ";
                    time = v.time;
                    cost = v.cost;
                }

                if(cost <= budget){
                    budget_paths.push_back(p);
                }

                cout<<"Cost: "<<cost<<" Time: "<<time<<endl;
                q.pop();
            }

            double time;
            for(auto p : budget_paths){
                for(auto v : p){
                    time = v.time;
                }

                if(time < best_time){
                    best_path = p;
                }
            }

            if(budget_paths.size()==0){
                cout<<"NO PATH FOR BUDGET"<<endl;
                return;
            }

            cout<<"THE BEST PATH: "<<endl;

                for(auto v : best_path){
                    cout<<id2name(v.id)<<" ";
                }
                cout<<endl;
        }

        void find_cpaths(int src, int dest, Path& labels, int &path_index, Path& path, priority_queue<Path, vector<Path>, Compare>& q){

            labels[src].state = FINISHED;
            path.push_back(labels[src]);
            path_index++;

            if (src == dest) {
                q.push(path);
            }else {
                for (auto neighbour : vertices[src].outgoing) {
                    if (labels[neighbour.vertex_id].state == UNDISCOVERED){
                        labels[neighbour.vertex_id].pred = src;
                        labels[neighbour.vertex_id].cost = labels[src].cost + neighbour.cost;
                        labels[neighbour.vertex_id].time = labels[src].time + neighbour.time;
                        find_cpaths(neighbour.vertex_id, dest, labels, path_index, path, q);
                    }
                }
            }
            labels[src].state = UNDISCOVERED;
            labels[src].pred = -1;
            labels[src].cost = 0.0;
            labels[src].time = 0.0;
            path_index--;
            path.pop_back();
            
        }


};