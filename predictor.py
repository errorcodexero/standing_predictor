#!/usr/bin/env python

import sys

def median(x): return sorted(x)[len(x)/2]

def quartiles(x):
	s=sorted(x)
	return max(x),x[len(x)*3/4],median(x),x[len(x)/4],min(x)

def flatten(a):
	r=[]
	for elem in a: r.extend(elem)
	return r

def parse_line(s):
	sp=s.split('\t')
	def parse_event(index):
		w2=sp[index].split()[1]
		if w2=='(IP)': return None

		x=sp[index].split()[0]
		try:
			d=int(x)
			#Events that haven't started yet are showing 0 pts.
			if d>0: return d
			return None
		except: return None
	def parse_qual(s):
		st=s.strip()
		if st=='Qualified': return 'in'
		if st=='---': return 'out'
		print 'not understood: \"%s\"'%s
		raise

	#print len(sp),sp
	return {
		'rank':sp[0],
		'total_points':int(sp[1]),
		'team':sp[2],
		'team_number':int(sp[2].split()[0]),
		'event1':parse_event(3),
		'event2':parse_event(4),
		'dcmp_qual':parse_qual(sp[5])
		}

def parse_file(input_filename):
	lines=file(input_filename).read().splitlines()
	return map(parse_line,lines[:])

#team_names={}
#for a in parse_file('march26.txt'):
#	t=a['team']
#	team_names[t.split()[0]]=t

def parse_line_tba(s):
	sp=s.split('\t')
	def parse_event(index):
		try:
			return int(sp[index])
		except:
			return None
	return {
		'rank':sp[0],
		'total_points':int(sp[6]),
		'team':team_names[sp[1].split()[0]],
		'event1':parse_event(2),
		'event2':parse_event(3)
		}

def main(input_filename,dcmp_event_size):
	lines=file(input_filename).read().splitlines()

	#m=map(parse_line_tba,lines[:])
	m=parse_file(input_filename)
	#for a in m:
	#	print a
		
	scores=filter(lambda x: x,map(lambda x: x['event1'],m)+map(lambda x: x['event2'],m))
	#for a in sorted(scores): print a
	#print 'Individual event score range:',quartiles(scores)#max(scores),median(scores),min(scores)

	max_pts_per_event=80#this is an assumption

	def max_pts(team_info):
		def get(name):
			if team_info[name] is None: return max_pts_per_event
			return team_info[name]
		return get('event1')+get('event2')
		
	l=list(reversed(sorted(map(max_pts,m))))
	min_cutoff_for_dcmp=m[dcmp_event_size-1]['total_points']
	print 'Best-case points to make it to district cmp:',min_cutoff_for_dcmp
	max_cutoff_for_dcmp=l[dcmp_event_size-1]
	print 'Worst-case points to make it to district cmp:',max_cutoff_for_dcmp

	average_points_per_event=22 #approx
	def expected_pts(team_info):
		def get(name):
			if team_info[name] is None:
				if team_info['event1'] is None:
					return average_points_per_event
				return team_info['event1']
			return team_info[name]
		return get('event1')+get('event2')

	l=list(reversed(sorted(map(expected_pts,m))))
	cutoff_best_est=l[dcmp_event_size-1]
	print 'Best estimate:',cutoff_best_est

	#3 events left, 
	#12*32*2 qualifi
	def points_per_event():
		quals=2*3*70
		elims=7*10*3
		awards=60
		return quals+elims+awards
	#for a in sorted(l): print a	
	#points_left=3*points_per_event()
	#print points_left
	#print points_left/32

	score_pairs=[]
	for x in m:
		if x['event1'] is None: continue
		if x['event2'] is None: continue
		score_pairs.append((x['event1'],x['event2']))
		score_pairs.append((x['event2'],x['event1']))

	def worst_of_at_least_as_good(score):
		#find all the teams that have one event that's at least that good and give back the worst performance that any of them have had
		#print 'score=',score
		f=filter(lambda x: x[0]>=score,score_pairs)
		if len(f):
			return min(map(lambda x: x[1],f))
		#if no teams who have played this well have competed more than once project their performance based on the best acheived by somebody how did play twice
		f=flatten(score_pairs)
		if len(f):
			return worst_of_at_least_as_good(max(f))

		#if no team has played twice, then assume they will get exactly the same score
		return score

	def best_of_this_or_worse(score):
		#the best performance that a team has had that's gotten a the score or better has gotten
		f=filter(lambda x: x[0]<=score,score_pairs)
		if len(f):
			return max(map(lambda x: x[1],f))
		#if no team that has played twice did this poorly then project the team based on the worst performance from a team that did play twice
		flat=flatten(score_pairs)
		if len(flat):
			return best_of_this_or_worse(min(flat))

		#If no team has played twice, then assume that will get exactly the same score
		return score

	def max_likely_points(team_info):
		if team_info['event2'] is not None: return team_info['total_points']
		if team_info['event1'] is None: return average_points_per_event*2
		return team_info['event1']+best_of_this_or_worse(team_info['event1'])
		
	def min_likely_points(team_info):
		if team_info['event2'] is not None: return team_info['total_points']
		x=team_info['event1']
		if x is None: return average_points_per_event*2
		return team_info['event1']+worst_of_at_least_as_good(team_info['event1'])
		
	def status(team_info):
		if max_pts(team_info)<min_cutoff_for_dcmp:
			return 'out'
		if team_info['total_points']>max_cutoff_for_dcmp:
			return 'in'
		if max_likely_points(team_info)<cutoff_best_est:
			return '~out'
		if min_likely_points(team_info)>cutoff_best_est:
			return '~in'
		return 'bubble'
		
	def count(a):
		m={}
		for elem in a:
			if not m.has_key(elem): m[elem]=0
			m[elem]+=1
		return m
		
	st=map(status,m)
	print count(st)
	for i,team in enumerate(m):
		print '%s\t'%i,'%s\t'%status(team),team['team']

if __name__=='__main__':
	from optparse import OptionParser
	p=OptionParser()
	p.add_option('--data',default='march11.txt')

	dcmp_sizes={
		#Updated for 2017
		#Chesapeake District Championship
		'CHS':58,
		#Indiana State Championship
		'IN':32,
		#Israel
		'ISR':45,
		#Michigan State Championship
		'FiM':160,
		#Mid-Atlantic Robotics District Championship
		'MAR':60,
		#North Carolina FIRST Robotics State Championship
		'NC':32,
		#New England District Championship
		'NE':64,
		#Ontario Provincial Championship
		'ONT':60,
		#Pacific Northwest District Championship
		'PNW':64,
		#Peachtree District State Championship
		'PCH':45
	#2015 district championship event sizes:
#		'FiM':102,
#		'IN':32,
#		'MAR':55,
#		'NE':60,
#		'PNW':64
		}
	p.add_option('--event_size',type='int')
	p.add_option('--district',help='Options:'+' '.join(dcmp_sizes))
	options,args=p.parse_args()
	assert len(args)==0
	if options.event_size:
		assert not options.district
		event_size=options.event_size
	else:
		if not options.district:
			options.district='PNW'
		event_size=dcmp_sizes[options.district]
	main(options.data,event_size)
