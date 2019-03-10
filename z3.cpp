#include<iostream>
#include<vector>
#include<algorithm>
#include "z3++.h"

using namespace std;
using namespace z3;

#define nyi { cout<<"nyi "<<__LINE__<<"\n"; exit(44); }
#define PRINT(X) cout<<""#X<<":"<<(X)<<"\n";

template<typename T>
vector<T> sorted(vector<T> a){
	std::sort(begin(a),end(a));
	return a;
}

template<typename T>
void print_lines(T t){
	for(auto elem:t){
		cout<<elem<<"\n";
	}
}

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
	return range(T{0},lim);
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

template<typename A,typename B>
vector<B> seconds(vector<pair<A,B>> a){
	vector<B> r;
	for(auto elem:a) r|=elem.second;
	return r;
}

template<typename A,typename B>
ostream& operator<<(ostream& o,pair<A,B> const& p){
	return o<<"("<<p.first<<","<<p.second<<")";
}

template<typename T>
T sum(vector<T> a){
	T r{0};
	for(auto elem:a){
		r+=elem;
	}
	return r;
}

template<typename T>
vector<T> tail(vector<T> a){
	vector<T> r;
	for(auto i:range(size_t{1},a.size())){
		r|=a[i];
	}
	return r;
}

//start program-specific code

using Team=int;

unsigned qual_pts(int event_size,int rank){
	//TODO: FIXME
	assert(rank>0);
	assert(rank<=event_size);
	assert(event_size>0);
	return event_size-rank;
}

void add_event(string event_code,vector<Team> teams,context &c,solver &s){
	auto prefix=event_code+"_";

	//qual
	auto team_ranks=mapf(
		[&](auto team){
			auto name=prefix+"rank_"+as_string(team);
			auto t=c.int_const(name.c_str());
			s.add(t>=1 && t<=int(teams.size()));
			return make_pair(team,t);
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
	declare_not_equal(seconds(team_ranks));

	auto ranks=range(size_t(1),1+teams.size());
	mapf(
		[&](auto const& rank_var){
			//PRINT(rank_var);
			auto x=c.int_const( (prefix+"qual_pts_"+as_string(rank_var.first)).c_str() );
			for(auto rank:ranks){
				s.add(x==int(qual_pts(teams.size(),rank)) || rank_var.second!=int(rank));
			}
			return 0;
		},
		team_ranks
	);

	auto decl_team_number=[&](string name){
		auto x=c.int_const(name.c_str());
		auto v=(x==teams[0]);
		for(auto a:tail(teams)){
			//v|=(x==a);
			v= v || (x==a);
		}
		s.add(v);
		return x;
	};

	//alliance selection
	vector<z3::expr> places;
	for(auto i:range(1,9)){
		places|=decl_team_number(prefix+"capt"+as_string(i));
		places|=decl_team_number(prefix+"a"+as_string(i)+"_p1");
		places|=decl_team_number(prefix+"a"+as_string(i)+"_p2");
	}
	declare_not_equal(places);
	
	//next, put in all the logic to make who can be captain correct

	//all the places exist
	//teams are all different

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
	add_event("2018wase",range(100,124),c,s);
	switch(s.check()){
		case unsat:
			cout<<"unsat\n";
			break;
		case sat:{
			cout<<"sat\n";
			model m=s.get_model();
			vector<pair<string,string>> data;
			for(unsigned i=0;i<m.size();i++){
				func_decl v=m[i];
				assert(v.arity()==0);//don't know why
				//cout<<v.name()<<" = "<<m.get_const_interp(v)<<"\n";
				data|=make_pair(as_string(v.name()),as_string(m.get_const_interp(v)));
			}
			print_lines(sorted(data));
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
