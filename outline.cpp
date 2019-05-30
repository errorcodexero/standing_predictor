/*goal: for the given team, can they make the district championship, and if it's possible but not certain, what are their odds of making it in, and if they have events left, how many points do they need to earn to get to different levels of probability?

for each team:
	make probability distribution of how many points they might get
combine these to make probability distribution of where the points cutoff will be
foreach team:
	make listing of odds of dcmp w/ each possible # of points still to earn left

initial version could do assuming that all teams are equal
later version could assume that teams have some sort of skill quantified by either their first event, or ELO, or something else.

how to do worlds odds:
hold over some of the data from calculating the make DCMP cutoffs to get distribution for that
do the same distribution of points as a district event -> not good because have larger size & more chairmans winners, etc.
TODO: Find table of # of chairman's winners per DCMP
Engineering inspiration (#?)
Rookie All-Star (probability that given out?)
district championship winners -> just assume that they would have enough points anyway?
*/

#include<fstream>
#include<sstream>
#include<set>
#include<random>
#include<iomanip>
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

template<typename T>
std::set<T>& operator|=(std::set<T>& a,T t){
	a.insert(t);
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

using namespace tba;

std::vector<std::string> split(std::string s){
	std::vector<std::string> r;
	std::stringstream ss;
	for(auto c:s){
		if(isblank(c)){
			if(ss.str().size()){
				r|=ss.str();
				ss.str("");
			}
		}else{
			ss<<c;
		}
	}
	if(ss.str().size()){
		r|=ss.str();
	}
	return r;
}

template<typename T>
std::string tag(std::string name,T body){
	std::stringstream ss;
	ss<<"<"<<name<<">"<<body<<"</"<<split(name).at(0)<<">";
	return ss.str();
}

template<typename A,typename B,typename C,typename D,typename E>
std::ostream& operator<<(std::ostream& o,std::tuple<A,B,C,D,E> const& t){
	o<<"(";
	o<<std::get<0>(t)<<" ";
	o<<std::get<1>(t)<<" ";
	o<<std::get<2>(t)<<" ";
	o<<std::get<3>(t)<<" ";
	o<<std::get<4>(t);
	return o<<")";
}

template<typename T>
std::string as_string(T t){
	std::stringstream ss;
	ss<<t;
	return ss.str();
}

template<typename T>
std::string join(std::vector<T> const& a){
	std::stringstream ss;
	for(auto elem:a){
		ss<<elem;
	}
	return ss.str();
}

template<typename A,typename B,typename C,typename D,typename E>
std::string join(std::tuple<A,B,C,D,E> const& t){
	std::stringstream ss;
	#define X(N) ss<<std::get<N>(t);
	X(0) X(1) X(2) X(3) X(4)
	#undef X
	return ss.str();
}

template<typename A,typename B,typename C,typename D,typename E,typename F>
std::string join(std::tuple<A,B,C,D,E,F> const& t){
	std::stringstream ss;
	#define X(N) ss<<std::get<N>(t);
	X(0) X(1) X(2) X(3) X(4) X(5)
	#undef X
	return ss.str();
}

template<typename T>
auto tr(T t){ return tag("tr",t); }

template<typename T>
auto td(T t){ return tag("td",t); }

template<typename Func,typename A,typename B,typename C,typename D>
auto mapf(Func f,std::tuple<A,B,C,D> t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3)
		#undef X
	);
}

template<typename Func,typename A,typename B,typename C,typename D,typename E>
auto mapf(Func f,std::tuple<A,B,C,D,E> t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3),G(4)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3),X(4)
		#undef X
	);
}

template<typename Func,typename A,typename B,typename C,typename D,typename E,typename F>
auto mapf(Func f,std::tuple<A,B,C,D,E,F> t)
	#define G(N) decltype(f(std::get<N>(t)))
	-> std::tuple<G(0),G(1),G(2),G(3),G(4),G(5)>
	#undef G
{
	return make_tuple(
		#define X(N) f(std::get<N>(t))
		X(0),X(1),X(2),X(3),X(4),X(5)
		#undef X
	);
}

#define MAP(F,X) mapf([&](auto a){ return (F)(a); },(X))

template<typename Func,typename T>
T filter_unique(Func f,std::vector<T> a){
	std::vector<T> found;
	for(auto elem:a){
		if(f(elem)){
			found|=elem;
		}
	}
	assert(found.size()==1);
	return found[0];
}

template<typename Func,typename A,typename B>
auto mapf(Func f,std::pair<A,B> p){
	return make_pair(
		f(p.first),
		f(p.second)
	);
}

template<typename A,typename B>
std::string join(std::pair<A,B> p){
	std::stringstream ss;
	ss<<p.first;
	ss<<p.second;
	return ss.str();
}

template<typename T>
std::string table(T body){ return tag("table",body); }

template<typename T>
auto h2(T t){ return tag("h2",t); }

template<typename T>
auto th(T t){ return tag("th",t); }
auto th1(std::string s){ return th(s); }

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<B,C,D,E> tail(std::tuple<A,B,C,D,E> const& t){
	return make_tuple(std::get<1>(t),std::get<2>(t),std::get<3>(t),std::get<4>(t));
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,std::vector<T> b){
	for(auto elem:b){
		a|=elem;
	}
	return a;
}

template<typename T>
std::vector<T> operator+(std::vector<T> a,std::tuple<T,T,T,T> t){
	a|=std::get<0>(t);
	a|=std::get<1>(t);
	a|=std::get<2>(t);
	a|=std::get<3>(t);
	return a;
}

template<typename A,typename B,typename C,typename D,typename E>
std::tuple<A,B,C,D,E> operator|(std::tuple<A> a,std::tuple<B,C,D,E> b){
	return make_tuple(
		std::get<0>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}

template<typename A1,typename A,typename B,typename C,typename D,typename E>
std::tuple<A1,A,B,C,D,E> operator|(std::tuple<A1,A> a,std::tuple<B,C,D,E> b){
	return make_tuple(
		std::get<0>(a),
		std::get<1>(a),
		std::get<0>(b),
		std::get<1>(b),
		std::get<2>(b),
		std::get<3>(b)
	);
}

std::string link(std::string url,std::string body){
	return tag("a href=\""+url+"\"",body);
}

template<typename T>
std::vector<std::pair<size_t,T>> enumerate_from(size_t start,std::vector<T> v){
	std::vector<std::pair<size_t,T>> r;
	for(auto elem:v){
		r|=make_pair(start++,elem);
	}
	return r;
}

std::string td1(std::string s){ return td(s); }

template<typename T>
std::vector<T> operator+(std::vector<T> a,T b){
	a|=b;
	return a;
}

template<typename A,typename B,typename C,typename D,typename E>
std::vector<B> seconds(std::vector<std::tuple<A,B,C,D,E>> v){
	std::vector<B> r;
	for(auto t:v){
		r|=std::get<1>(t);
	}
	return r;
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

map<Point,Pr> operator+(map<Point,Pr> a,int i){
	map<Point,Pr> r;
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

set<Team_key> chairmans_winners(Cached_fetcher& f,District_key district){
	set<Team_key> r;
	for(auto event:district_events(f,district)){
		switch(event.event_type){
			case Event_type::DISTRICT_CMP:
			case Event_type::DISTRICT_CMP_DIVISION:
				continue;
			case Event_type::DISTRICT:
				break;
			default:
				PRINT(event.event_type);
				nyi
		}
		auto k=event.key;
		auto aw=event_awards(f,k);
		if(aw.empty()) continue;
		auto f=filter_unique([](auto a){ return a.award_type==Award_type::CHAIRMANS; },aw);
		if(f.recipient_list.size()!=1){
			PRINT(f);
		}
		assert(f.recipient_list.size()==1);
		auto team=f.recipient_list[0].team_key;
		assert(team);
		r|=*team;
	}
	return r;
}

string make_link(Team_key team){
	auto s=team.str();
	assert(s.substr(0,3)=="frc");
	auto t=s.substr(3,500);
	return link("https://www.thebluealliance.com/team/"+t,t);
}

	auto digit=[](auto i)->char{
		if(i<10) return '0'+i;
		return 'a'+(i-10);
	};

string color(double d){
	//input range 0-1
	//red->white->green

	auto f=[](double v){
		auto x=min(255,int(255*v));
		return string()+digit(x>>4)+digit(x&0xf);
	};

	auto rgb=[=](double r,double g,double b){
		return "#"+f(r)+f(g)+f(b);
	};

	if(d<.5){
		return rgb(1,d*2,d*2);
	}
	auto a=2*(1-d);
	return rgb(a,1,a);
}

string colorize(double d){
	return tag("td bgcolor=\""+color(d)+"\"",
		[&](){
			stringstream ss;
			ss<<setprecision(3)<<fixed;
			ss<<d;
			return ss.str();
		}()
	);
}

double entropy(Pr p){
	//units are bits.
	if(p<0) p=0;
	if(p>1) p=1;
	if(p==0 || p==1) return 0;
	assert(p>0 && p<1);
	return -(log(p)*p+log(1-p)*(1-p))/log(2);
}

using Extended_cutoff=pair<Point,Pr>;

map<Point,Pr> simplify(map<pair<Point,Pr>,Pr> m){
	map<Point,Pr> r;
	for(auto [k,v]:m){
		r[k.first]+=v;
	}
	return r;
}

string gen_html(
	vector<tuple<Team_key,Pr,Point,Point,Point>> result,
	vector<Team> team_info,
	map<Extended_cutoff,Pr> cutoff_pr,
	string title,
	string district_short,
	Year year,
	int dcmp_size
){
	//auto title="PNW District Championship Predictions 2019"; //TODO: Put in date & make district configurable
		//cout<<"Team #\tP(DCMP)\tPts 5%\tPts 50%\tPts 95%\tNickname\n";

	auto nickname=[&](auto k){
		auto f=filter_unique([=](auto a){ return a.key==k; },team_info);
		auto v=f.nickname;
		assert(v);
		return *v;
	};

	auto cutoff_table=[=](){
		return h2("Cutoff value")+tag("table border",
			tr(th("Points")+th("Probability"))+
			join(mapf(
				[](auto a){
					return tr(join(MAP(td,a)));
				},
				simplify(cutoff_pr)
			))
		);
	}();

	auto cutoff_table_long=[=](){
		return h2("Cutoff value - extended")+
		"The cutoff values, along with how likely a team at that value is to miss advancing.  For example: a line that said (50,.25) would correspond to the probability that team above 50 get in, teams below 50 do not, and 75% of teams ending up with exactly 50 would qualify for the district championship."+
		tag("table border",
			tr(th("Points")+th("Probability"))+
			join(mapf(
				[](auto a){
					return tr(join(MAP(td,a)));
				},
				cutoff_pr
			))
		);
	}();

	double total_entropy=sum(mapf(entropy,seconds(result)));
	PRINT(total_entropy);
	
	return tag("html",
		tag("head",
			tag("title",title)
		)+
		tag("body",
			tag("h1",title)+
			link("https://frc-events.firstinspires.org/2019/district/"+district_short,"FRC Events")+"<br>"+
			link("https://www.thebluealliance.com/events/"+district_short+"/"+as_string(year)+"#rankings","The Blue Alliance")+"<br>"+
			link("http://frclocks.com/index.php?d="+district_short,"FRC Locks")+"(slow)<br>"+
			"Slots at event:"+as_string(dcmp_size)+
			cutoff_table+
			h2("Team Probabilities")+
			tag("table border",
				tr(join(mapf(
					th1,
					std::vector<string>{
						"Probability rank",
						"Probability of making district championship",
						"Team number",
						"Nickname",
						"Extra points needed to have 5% chance of making district championship",
						"Extra points needed to have 50% chance of making district championship",
						"Extra points needed to have 95% chance of making district championship"
					}
				)))+
				join(
					mapf(
						[=](auto p){
							auto [i,a]=p;
							return tr(join(
								vector<string>{}+td(i)+
								colorize(get<1>(a))+
							mapf(
								td1,
								std::vector<std::string>{
									make_link(get<0>(a)),
									nickname(get<0>(a)),
									as_string(get<2>(a)),
									as_string(get<3>(a)),
									as_string(get<4>(a))
								}
							)));
						},
						enumerate_from(1,reversed(sorted(
							result,
							[](auto x){ return make_pair(get<1>(x),x); }
						)))
					)
				)
			)+
			cutoff_table_long
		)
	);
}

void run(Cached_fetcher &f,District_key district,Year year,int dcmp_size,string title,string district_short){
	vector<District_key> old_keys{
		//excluding 2014 since point system for quals was different.
		District_key{"2015pnw"},
		District_key{"2016pnw"},
		District_key{"2017pnw"},
		District_key{"2018pnw"},
		District_key{"2019pnw"}
	};

	auto team_info=district_teams(f,district);

	//print_lines(district_rankings(f,district));
	auto d=district_rankings(f,district);
	assert(d);
	auto d1=*d;
	//print_lines(d1);
	multiset<Point> old_results;
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
		//auto events_left=2-team.event_points.size();
		size_t max_counters=2-min(2,(int)team.event_points.size()); //min in case district championship has already been played.
		auto events_scheduled=team_events_year_keys(f,team.team_key,year);
		auto events_left=min(max_counters,events_scheduled.size()-team.event_points.size());
		assert(events_left>=0);
		by_team[team.team_key]=[&]()->map<Point,Pr>{
			auto first_event_points=[=]()->double{
				if(team.event_points.size()){
					return team.event_points[0].total;
				}
				return 0;
			}();
			if(events_left==0){
				return map<Point,Pr>{{
					first_event_points+[=]()->double{
						if(team.event_points.size()>1){
							return team.event_points[1].total;
						}
						return 0;
					}(),
					1
				}};
			}
			if(events_left==1){
				return to_map(mapf(
					[&](auto p){
						return make_pair(int(p.first+first_event_points),p.second);
					},
					pr
				));
			}
			if(events_left==2){
				return convolve(pr,pr)+team.rookie_bonus;
			}
			PRINT(team);
			PRINT(events_left);
			nyi
		}()+team.rookie_bonus;
	}

	print_lines(by_team);
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

	auto teams_advancing=dcmp_size-chairmans.size();
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

	multiset<pair<Point,Pr>> cutoffs;
	const auto iterations=2000; //usually want this to be like 2k
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
				if(total>teams_left_out){
					auto excess=total-teams_left_out;
					return make_pair(points,1-excess/teams);
				}else if(total==teams_left_out){
					//to make it so that being at cutoff number is min to possibly be in, rather than 
					//saying cutoff is X but everyone at that number missed.
					return make_pair(points+1,0.0);
				}
			}
			assert(0);
		}();
		//PRINT(find_cutoff);
		cutoffs|=find_cutoff;
	}

	//print_lines(count(cutoffs));
	map<pair<Point,Pr>,Pr> cutoff_pr=map_values(
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
	map<Pr,Extended_cutoff> interesting_cutoffs;
	for(auto d:{.05,.5,.95}){
		interesting_cutoffs[d]=cutoff_level(d);
	}

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
				if(team_value>cutoff.first){
					pr_make+=combined_pr;
				}else if(team_value==cutoff.first){
					pr_make+=combined_pr*(1-cutoff.second);
					pr_miss+=combined_pr*cutoff.second;
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
			PRINT(total);
			assert(total>.99 && total<1.01);

			//PRINT(pr_make);
			//points needed to have 50% odds; 5% odds; 95% odds, or quartiles?
			auto points_so_far=team.point_total;
			vector<Point> interesting;
			for(auto [pr,pts]:interesting_cutoffs){
				//cout<<pr<<":"<<max(0.0,pts-points_so_far)<<"\n";
				auto value=max(0,int(pts.first-points_so_far));
				interesting|=value;
			}
			assert(interesting.size()==3);
			result|=make_tuple(team.team_key,pr_make,interesting[0],interesting[1],interesting[2]);
		}
	}

	auto x=mapf([](auto x){ return get<1>(x); },result);
	PRINT(sum(x));

	{
		auto g=gen_html(result,team_info,cutoff_pr,title,district_short,year,dcmp_size);
		ofstream f(district.get()+".html");
		f<<g;
	}

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
}

#if 0
void dcmp_awards(District_key district){
	map<District_key,int> chairmans{
		{"2019pnw",3/*chairmans*/+2/*ei*/+1/*ras*/},
	};
	//able to win DCMP EI without competing there with robot?
	//looks like 568 did last year in PNW
}
#endif

int cmp_slots(District_key district){
	map<string,int> slots{
		{"2019chs",21},
		{"2019fim",87},
		{"2019isr",11},
		{"2019fma",21},
		{"2019in",10},
		{"2019ne",33},
		{"2019ont",29},
		{"2019tx",38},
		{"2019fnc",15},
		{"2019pnw",31},
		{"2019pch",17},
	};
	auto f=slots.find(district.get());
	assert(f!=slots.end());
	return f->second;
}

vector<string> args(int argc,char **argv){
	vector<string> r;
	//ignore program name
	for(int i=1;i<argc;i++){
		r|=string{argv[i]};
	}
	return r;
}

int main(int argc,char **argv){
	auto a=args(argc,argv);
	//first, look up teams in the district
	//for each team
		//look up their schedule
		//look up points earned already
	auto tba_key=slurp("../tba/auth_key");
	Cached_fetcher f{Fetcher{Nonempty_string{tba_key}},Cache{}};
	Year year{2019};
	//PRINT(districts(f,year));
	auto dists=[&](){
		/*if(a.size()){
			for(auto elem:a){
				if(elem=="--help" || elem=="-h"){
					cout<<"Create HTML output giving district championship odds.\n";
					cout<<"args: optionally, a list of district keys.\n";
					exit(0);
				}
			return a;
		}*/
		return districts(f,year);
	}();
	for(auto year_info:dists){
		//District_key district{"2019pnw"};
		auto district=year_info.key;
		PRINT(district);
		//if( !(district==District_key{"2019pnw"}) ) continue;
		auto dcmp_size=[=](){
			if(district=="2019chs") return 58;
			if(district=="2019isr") return 45;
			if(district=="2019fma") return 60;
			if(district=="2019fnc") return 32;
			if(district=="2019ont") return 80;
			if(district=="2019tx") return 64;
			if(district=="2019in") return 32;
			if(district=="2019fim") return 160;
			if(district=="2019ne") return 64;
			if(district=="2019pnw") return 64;
			if(district=="2019pch") return 45;
			nyi
		}();
		auto title=year_info.display_name+" District Championship Predictions "+as_string(year);
		run(f,district,year,dcmp_size,title,year_info.abbreviation);
	}
	return 0;
}
