/*goal: for the given team, can they make the district championship, and if it's possible but not certain, what are their odds of making it in, and if they have events left, how many points do they need to earn to get to different levels of probability?

for each team:
	make probability distribution of how many points they might get
combine these to make probability distribution of where the points cutoff will be
foreach team:
	make listing of odds of dcmp w/ each possible # of points still to earn left

initial version could do assuming that all teams are equal
later version could assume that teams have some sort of skill quantified by either their first event, or ELO, or something else.
*/

#include<fstream>
#include<sstream>
#include<set>
#include<random>
#include "tba/db.h"
#include "tba/data.h"
#include "tba/tba.h"
#include "tba/util.h"

//start generic stuff

#define PRINT TBA_PRINT
#define nyi TBA_NYI

std::string slurp(std::string const& filename){
    std::ifstream f(filename.c_str());
    if(!f.good()){
        throw "File_not_found(filename)";
    }
    std::stringstream ss;
    while(f>>ss.rdbuf());
    return ss.str();
}

template<typename T>
void print_lines(T t){
	for(auto elem:t){
		std::cout<<elem<<"\n";
	}
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,T t){
	a.insert(t);
	return a;
}

template<typename T>
std::multiset<T>& operator|=(std::multiset<T>& a,std::multiset<T> b){
	for(auto elem:b){
		a|=elem;
	}
	return a;
}

template<typename K,typename V>
std::vector<V> seconds(std::map<K,V> a){
	std::vector<V> r;
	for(auto elem:a){
		r|=elem.second;
	}
	return r;
}

double sum(std::vector<double> v){
	double r=0;
	for(auto elem:v){
		r+=elem;
	}
	return r;
}

template<typename Func,typename T>
auto mapf(Func f,std::vector<T> const& v)->std::vector<decltype(f(v[0]))>{
	std::vector<decltype(f(v[0]))> r;
	for(auto elem:v){
		r|=f(elem);
	}
	return r;
}

template<typename Func,typename K,typename V>
auto mapf(Func f,std::map<K,V> const& v)->std::vector<decltype(f(*std::begin(v)))>{
	std::vector<decltype(f(*std::begin(v)))> r;
	for(auto p:v){
		r|=f(p);
	}
	return r;
}

template<typename K,typename V>
std::map<K,V> to_map(std::vector<std::pair<K,V>> v){
	std::map<K,V> r;
	for(auto p:v){
		auto f=r.find(p.first);
		assert(f==r.end());
		r[p.first]=p.second;
	}
	return r;
}

template<typename K,typename V>
auto values(std::map<K,V> a)->std::vector<V>{
	std::vector<V> r;
	for(auto p:a) r|=p.second;
	return r;
}

template<typename T>
std::map<T,size_t> count(std::multiset<T> const& a){
	std::map<T,size_t> r;
	for(auto elem:a){
		r[elem]=a.count(elem); //slow
	}
	return r;
}

template<typename Func,typename K,typename V>
auto map_values(Func f,std::map<K,V> m)->std::map<K,decltype(f(begin(m)->second))>{
	std::map<K,decltype(f(begin(m)->second))> r;
	for(auto [k,v]:m){
		r[k]=f(v);
	}
	return r;
}

template<typename T,typename Func>
std::vector<T> sorted(std::vector<T> v,Func f){
	sort(begin(v),end(v),[&](auto a,auto b){ return f(a)<f(b); });
	return v;
}

template<typename T>
std::vector<T> reversed(std::vector<T> a){
	reverse(begin(a),end(a));
	return a;
}

//start program-specific stuff.

using namespace std;
using namespace tba;

using Pr=double; //probability
using Point=int;

multiset<Point> point_results(Cached_fetcher& server,District_key dk){
	auto d=district_rankings(server,dk);
	assert(d);
	multiset<Point> r;
	for(auto team_result:*d){
		for(auto event:team_result.event_points){
			r|=Point(event.total);
		}
	}
	return r;
}

map<Point,Pr> convolve(map<Point,Pr> a,map<Point,Pr> b){
	map<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

template<typename Func,typename T>
T filter_unique(Func f,vector<T> a){
	vector<T> found;
	for(auto elem:a){
		if(f(elem)){
			found|=elem;
		}
	}
	assert(found.size()==1);
	return found[0];
}

map<Point,Pr> operator+(map<Point,Pr> a,int i){
	map<Point,Pr> r;
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

template<typename T>
set<T>& operator|=(set<T>& a,T t){
	a.insert(t);
	return a;
}

set<Team_key> chairmans_winners(Cached_fetcher& f,District_key district){
	set<Team_key> r;
	for(auto event:district_events(f,district)){
		auto k=event.key;
		auto aw=event_awards(f,k);
		if(aw.empty()) continue;
		auto f=filter_unique([](auto a){ return a.award_type==Award_type::CHAIRMANS; },aw);
		assert(f.recipient_list.size()==1);
		auto team=f.recipient_list[0].team_key;
		assert(team);
		r|=*team;
	}
	return r;
}

int main(int argc,char **argv){
	//first, look up teams in the district
	//for each team
		//look up their schedule
		//look up points earned already
	auto tba_key=slurp("../tba/auth_key");
	Cached_fetcher f{Fetcher{Nonempty_string{tba_key}},Cache{}};
	District_key district{"2019pnw"};

	auto team_info=district_teams(f,district);

	//print_lines(district_rankings(f,district));
	auto d=district_rankings(f,district);
	assert(d);
	auto d1=*d;
	//print_lines(d1);
	multiset<Point> old_results;
	vector<District_key> old_keys{
		//excluding 2014 since point system for quals was different.
		District_key{"2015pnw"},
		District_key{"2016pnw"},
		District_key{"2017pnw"},
		District_key{"2018pnw"},
		District_key{"2019pnw"}
	};
	for(auto key:old_keys){
		old_results|=point_results(f,district);
	}
	map<Point,unsigned> occurrances;
	for(auto value:old_results){
		occurrances[value]=old_results.count(value); //slow
	}
	map<Point,Pr> pr;
	for(auto [pts,count]:occurrances){
		pr[pts]=double(count)/old_results.size();
	}
	//print_lines(pr);
	//PRINT(sum(seconds(pr)));
	//PRINT(old_results.size())

	auto chairmans=chairmans_winners(f,district);

	map<Team_key,map<Point,Pr>> by_team;
	for(auto team:d1){
		if(chairmans.count(team.team_key)){
			continue;
		}
		auto events_left=2-team.event_points.size();
		assert(events_left>=0);
		by_team[team.team_key]=[&]()->map<Point,Pr>{
			if(events_left==0){
				return map<Point,Pr>{{
					team.event_points[0].total+team.event_points[1].total,
					1
				}};
			}
			if(events_left==1){
				return to_map(mapf(
					[&](auto p){
						return make_pair(int(p.first+team.event_points[0].total),p.second);
					},
					pr
				));
			}
			if(events_left==2){
				return convolve(pr,pr)+team.rookie_bonus;
			}
			PRINT(team);
			nyi
		}()+team.rookie_bonus;
	}

	//print_lines(by_team);
	bool by_team_csv=0;
	if(by_team_csv){
		cout<<"team,";
		for(auto i:range(140)){
			cout<<i<<",";
		}
		for(auto [team,data]:by_team){
			cout<<team<<",";
			for(auto i:range(140)){
				auto f=data.find(i);
				if(f==data.end()){
					cout<<"0,";
				}else{
					cout<<f->second<<",";
				}
			}
			cout<<"\n";
		}
	}

	map<Point,Pr> by_points; //# of teams expected to end at each # of points
	for(auto [team,data]:by_team){
		(void)team;
		for(auto [pts,pr]:data){
			auto f=by_points.find(pts);
			if(f==by_points.end()){
				by_points[pts]=pr;
			}else{
				f->second+=pr;
			}
		}
	}

	bool sum_display=0;
	if(sum_display){
		for(auto [pts,pr]:by_points){
			cout<<pts<<","<<pr<<"\n";
		}
	}

	map<Point,Pr> cdf;
	{
		double d=0;
		for(auto [pts,pr]:by_points){
			d+=pr;
			cdf[pts]=d;
		}
	}

	bool cdf_display=0;
	if(cdf_display){
		for(auto [pts,pr]:cdf){
			cout<<pts<<","<<pr<<"\n";
		}
	}

	auto teams_advancing=64-chairmans.size();
	auto teams_competing=sum(values(by_points));
	auto teams_left_out=teams_competing-teams_advancing;

	//monte carlo method for where the cutoff is

	std::mt19937_64 rng;
	// initialize the random number generator with time-dependent seed
	uint64_t timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::seed_seq ss{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
	rng.seed(ss);
	// initialize a uniform distribution between 0 and 1
	std::uniform_real_distribution<double> unif(0, 1);

	auto sample=[&](map<Point,Pr> const& m)->Point{
		auto num=unif(rng);
		double total=0;
		for(auto [value,pr]:m){
			total+=pr;
			if(total>num){
				return value;
			}
		}
		assert(0);
	};

	multiset<Point> cutoffs;
	const auto iterations=2000;
	for(auto iteration:range(iterations)){
		//PRINT(iteration);
		map<Point,unsigned> final_points;
		for(auto [team,dist]:by_team){
			final_points[sample(dist)]++;
		}
		auto find_cutoff=[&](){
			unsigned total=0;
			for(auto [points,teams]:final_points){
				total+=teams;
				if(total>=teams_left_out){
					return points;
				}
			}
		}();
		//PRINT(find_cutoff);
		cutoffs|=find_cutoff;
	}

	//print_lines(count(cutoffs));
	map<Point,Pr> cutoff_pr=map_values(
		[=](auto x){ return (0.0+x)/iterations; },
		count(cutoffs)
	);
	print_lines(cutoff_pr);

	auto cutoff_level=[=](Pr probability_target){
		double t=0;
		for(auto [points,pr]:cutoff_pr){
			t+=pr;
			if(t>probability_target){
				return points;
			}
		}
		assert(0);
	};
	map<Pr,Point> interesting_cutoffs;
	for(auto d:{.05,.5,.95}){
		interesting_cutoffs[d]=cutoff_level(d);
	}
	//double p50_cutoff=cutoff_level(.5);
	//double p95_cutoff=cutoff_level(.95);

	/*
	TODO: Rookie points, chairman's award
	*/

	vector<tuple<Team_key,Pr,Point,Point,Point>> result;
	for(auto team:d1){
		//PRINT(team);
		//PRINT(team.team_key);
		//probability that get in
		//subtract the cutoff pr
		auto team_pr=by_team[team.team_key];
		double pr_make=0;
		double pr_miss=0;
		for(auto [cutoff,c_pr]:cutoff_pr){
			for(auto [team_value,t_pr]:team_pr){
				auto combined_pr=c_pr*t_pr;
				if(team_value>cutoff){
					pr_make+=combined_pr;
				}else{
					pr_miss+=combined_pr;
				}
			}
		}
		//PRINT(pr_make+pr_miss);
		auto total=pr_make+pr_miss;
		if(chairmans.count(team.team_key)){
			assert(total>=-.01 && total<=.01);
			result|=make_tuple(team.team_key,1.0,0,0,0);
		}else{
			assert(total>.99 && total<1.01);

			//PRINT(pr_make);
			//points needed to have 50% odds; 5% odds; 95% odds, or quartiles?
			auto points_so_far=team.point_total;
			vector<Point> interesting;
			for(auto [pr,pts]:interesting_cutoffs){
				//cout<<pr<<":"<<max(0.0,pts-points_so_far)<<"\n";
				auto value=max(0,int(pts-points_so_far));
				interesting|=value;
			}
			assert(interesting.size()==3);
			result|=make_tuple(team.team_key,pr_make,interesting[0],interesting[1],interesting[2]);
		}
	}

	auto x=mapf([](auto x){ return get<1>(x); },result);
	PRINT(sum(x));

	bool show_table=1;
	if(show_table){
		cout<<"Team #\tP(DCMP)\tPts 5%\tPts 50%\tPts 95%\tNickname\n";
		cout.precision(3);
		for(auto a:reversed(sorted(
			result,
			[](auto x){ return make_pair(get<1>(x),x); }
		))){
			cout<<get<0>(a).str().substr(3,100)<<"\t";
			cout<<get<1>(a)<<"\t";
			cout<<get<2>(a)<<"\t";
			cout<<get<3>(a)<<"\t";
			cout<<get<4>(a)<<"\t";
			auto f=filter_unique([=](auto f){ return f.key==get<0>(a); },team_info);
			cout<<f.nickname<<"\n";
		}
	}

	return 0;
}
