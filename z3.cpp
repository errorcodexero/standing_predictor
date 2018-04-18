#include<iostream>
#include<vector>
#include "z3++.h"

using namespace std;
using namespace z3;

#define nyi { cout<<"nyi "<<__LINE__<<"\n"; exit(44); }

template<typename T>
vector<T>& operator|=(vector<T> &a,T t){
	a.push_back(t);
	return a;
}

template<typename T>
vector<T> range(T start,T lim){
	vector<T> r;
	for(auto i=start;i<lim;i++){
		r|=i;
	}
	return r;
}

template<typename T>
vector<T> range(T lim){
	return range(0,lim);
}

template<typename Func,typename T>
auto mapf(Func f,vector<T> v){
	vector<decltype(f(v[0]))> r;
	for(auto a:v){
		r|=f(a);
	}
	return r;
}

template<typename T>
string as_string(T t){
	stringstream ss;
	ss<<t;
	return ss.str();
}

//start program-specific code

using Team=int;

unsigned qual_pts(int event_size,int rank){
	nyi
}

void add_event(string event_code,vector<Team> teams,context &c,solver &s){
	auto prefix=event_code+"_";

	//qual
	auto team_ranks=mapf(
		[&](auto team){
			auto t=c.int_const((prefix+"rank_"+as_string(team)).c_str()); 
			s.add(t>=1 && t<=int(teams.size()));
			return t;
		},
		teams
	);
	auto declare_not_equal=[&](auto a){
		for(auto i:range(a.size())){
			for(auto j:range(i)){
				s.add(a[i]!=a[j]);
			}
		}
	};
	declare_not_equal(team_ranks);

	/*mapf(
		[&](auto const& rank_var){

			auto x=c.int_const(prefix+"qual_pts_"+team);
			s.add(x==sum(
				mapf(
					[](auto i){
						return (rank_var==i)?qual_pts(teams.size(),i):0;
					},
					ranks
				)
			));
		},
		team_ranks
	);*/

	//alliance selection
	

	//elims
	//awards
	//total points
}

void demo(){
	/*for each of the events:
	 * add event
	 *for each of the events that has finished:
	 *   add details about how it went
	 *   including who's no longer eligible for chairmans
	 *after have all of the constraints, need to run it with the push/pop
	 logic that will ask each of the interesting questions
	 * */
	context c;
	solver s(c);
	add_event("2018wase",range(100,105),c,s);
	switch(s.check()){
		case unsat:
			cout<<"unsat\n";
			break;
		case sat:{
			cout<<"sat\n";
			model m=s.get_model();
			for(unsigned i=0;i<m.size();i++){
				func_decl v=m[i];
				assert(v.arity()==0);
				cout<<v.name()<<" = "<<m.get_const_interp(v)<<"\n";
			}
			break;
		}
		case unknown:
			cout<<"Unknown\n";
			break;
		default:
			assert(0);
	}
}

int main(){
	demo();
}
