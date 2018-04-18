#!/usr/bin/env python

#This program generates input to the Z3 theorem prover.  
#Note that it just outputs text; it should instead get replaced by something that actually uses one of the z3 language binding libraries.

"""
constraints:
-awards given out via the rules
  -0 or 1 of most things at district events
  -special numbers given out at DCMP
  -5 pts for judged team awards
  -most awards must go to different teams, but not safety
  -rookies (& have they already won Rookie All-Star)
  -who is qualified to win chairmans award?
-ranking points given out to teams at different events
  -no teams rank the same at a given event
-alliance selection points
  -no teams chosen at the same point
-playoff perf points
  -backup robots
  -max numbers of matches won at each level of playoffs at each event
    -rules to know how many matches there were played

Questions to ask:
What are the min & max values that the cutoff might be?
Is it possible for team X to make it the DCMP? to worlds?
What is the worst that team X can do in upcoming events to still be able to make it?
What is the worst that team X can do to gauruntee that they do make it?
(answer w/ or without assuming that might win chairmans/other auto qual)

First step:
Assume that have only like 3 teams and want to know top 1
Assume that only one event ever

Output format:
1) Green=qualified
2) Red=cannot qualify
3) Orange=cannot qualify on points
4) White=Can qualify on points

#RP points
q1_index in 1-3
q2_index in 1-3
q3_index in 1-3
q1_index!=q2_index
q1_index!=q3_index
q2_index!=q3_index

#or better to forumulate as:
rank_<TEAM> in 1-<# OF TEAMS>

#Point totals
t1_points=(q1_index==1)?3:((q2_index==1)?2:1)
t2_points=(q1_index==2)?3:((q2_index==2)?2:1)
t3_points=(q1_index==3)?3:((q2_index==3)?2:1)

#Alliance selection results
capt1=q1_index
r1_p1!=capt1
capt2!=capt1
capt2!=r1_p1
#...
capt2==q2_index || (r1_p1==q2_index && capt2==q3_index)
r1_p2!=...
#max unused rank for captain
rank(capt3)>rank(capt2) && rank(capt3)
rank(pickX)>rank(captX)

#boolean for each of the teams being in a particluar place?
award_eng_exc in teamnumbers|None
award_spirit in teamnumbers|None
award_eng_exc!=award_spirit
...

award_points_t1=((award_eng_exc==1)?5:0)+((award_eng_exc==1)?5:0)
award_points_t2=...

t1_is_rookie=0
t2_is_rookie=1
rookies=...
rookie_all_star in rookies|None

#Most point calculation
t1_wins= t1_points>t2_points && t1_points>t3_points
t2_wins= t2_points>t1_points && t2_points>t3_points
t3_wins= t3_points>t1_points && t3_points>t2_points

"""

def unique_pairs(a):
    #any(a) => [a] -> [(a,a)]
    out=[]
    for i in range(len(a)):
        for j in range(i):
            out.append((a[i],a[j]))
    return out


def get_qual_pts(rank):
    #TODO: Use the real numbers
    return 64-rank


def main():
    #TODO: Split this into per-event vars, which must be prefixed, and overall district vars
    #TODO: See about other ways to run the program since the text input is getting to be too large.
    #TODO: Add in the loops to show the min and max ranks that a team can get.

    def declare_int(name):
        #str
        print '(declare-const '+name+' Int)'

    def declare_not_equal(names):
        #[str]->void
        for i in range(len(names)):
            for j in range(i):
                print '(assert (not (= '+names[i]+' '+names[j]+')))'

    teams=[901,903,904,7000]+range(10)
    rookie_teams=[7000]

    def declare_from_list(name,options):
        #[int]->void
        declare_int(name)
        print '(assert (or',
        print ' '.join(map(lambda x: '(= '+name+' '+str(x)+')',options)),
        print '))'

    def declare_team_num(name):
        #Create a variable that must be a team number from the event.
        declare_from_list(name,teams)
        
    qual_ranks=range(1,1+len(teams))
    qual_rank_vars=map(lambda qual_rank: 'qual_rank'+str(qual_rank),qual_ranks)
    map(declare_team_num,qual_rank_vars)
    declare_not_equal(qual_rank_vars) #Same team does not have more than one rank

    #Awards
    standard=[
        'eng_ex',
        'ind_design',
        #TODO: rest of the team awards
        ]
    rookie_awards=['rookie_all_star','rookie_inspiration'] #high_rookie_seed is not judged.
    other=[
        'chairmans',
        'ei',
        'safety'
        ]

    all_team_awards=standard+other #TODO: Restrictions on chairmans winners
    all_awards=all_team_awards+rookie_awards

    def award_pts(award):
        #str->int

        if award in standard: return 5
        if award == 'chairmans': return 10
        if award == 'ei': return 8
        if award == 'safety': return 5
        if award in rookie_awards: return 5
        raise Exception('Unrecognzied award: %s'%award)

    for award in all_team_awards:
        declare_from_list('award_'+str(award),teams+[0]) #0=award was not given

    for award in rookie_awards:
        declare_from_list('award_'+str(award),rookie_teams+[0])

    nonduplicate_awards=standard+rookie_awards+['chairmans','ei']
    for i in range(len(nonduplicate_awards)):
        for j in range(i):
            a1='award_'+nonduplicate_awards[i]
            a2='award_'+nonduplicate_awards[j]
            print '(assert (or (= '+a1+' 0) (not (= '+a1+' '+a2+'))))'


    #Total points at the event
    map(lambda x: declare_int('points_'+str(x)),teams)

    for team in teams:
        print '(assert (= points_'+str(team),
        print '(+ ',

        #Ranking points
        print ' '.join(map(lambda qual_rank: '(ite (= qual_rank'+str(qual_rank)+' '+str(team)+') '+str(get_qual_pts(qual_rank))+' 0)',qual_ranks)),

        #Award points
        print ' '.join(map(lambda award: '(ite (= award_'+award+' '+str(team)+') '+str(award_pts(award))+' 0)',all_awards))

        print ')',
        print '))'

    #TODO: Put in district rank logic.
    #TODO: Figure out how to do all of the tiebreakers?
    #Or just leave it so that a team is always ranked above those with fewer points and below those with more?
    #Or put in extra vars for the tiebreakers and if they're not known, then make the system figure them out.

    for team in teams:
        v='district_rank_'+str(team) # number of teams that must be ranked above them
        declare_int(v)

        #District rank must be at least as high as 1+ # of teams w/ more district points 
        print '(assert (>= '+v+' (+ 1 ',
        other_teams=filter(lambda x: x!=team,teams)
        print ' '.join(map(lambda x: '(ite (> points_'+str(x)+' points_'+str(team)+') 1 0)',other_teams)),
        print ')))'

        #District rank must be better than all of those with fewer points.
        #or in other words, no further down than those with more or at least as many points
        print '(assert (<= '+v+' (+ 1 ',
        other_teams=filter(lambda x: x!=team,teams)
        print ' '.join(map(lambda x: '(ite (>= points_'+str(x)+' points_'+str(team)+') 1 0)',other_teams)),
        print ')))'

    #All the district rankings should be different.
    declare_not_equal(map(lambda x: 'district_rank_'+str(x),teams))

    #for team in teams:
        #v='min_rank_'+str(team) #max number of teams that could be ranked above them

    #print '(assert (> rank1 3))'
    #print '(assert (
    print '(check-sat)'
    print '(get-model)'


if __name__=='__main__':
    main()
