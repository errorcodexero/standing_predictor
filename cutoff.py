#!/usr/bin/env python

#Look at historical qualification data to figure out cutoffs.

from argparse import ArgumentParser

def parse_line(s):
	sp=s.split('\t')
	def get(i): return sp[i].strip()
	def get_int(i): return int(get(i))

	def get_event(i):
		#returns maybe int
		s=get(i)
		if s=='Not Played':
			return None

		return int(s.split()[0])

	def get_d(i):
		s=get(i)
		if s=='---': return False
		return True

	return {
		'rank':get_int(0),
		'district_points':get(1),
		'team':get(2),
		'event1':get_event(3),
		'event2':get_event(4),
		'district_championship':get_d(5),
		'frc_championship':get(6),
		'team_age_points':get_int(7)
		}

def parse_file(input_filename):
	return map(parse_line,open(input_filename,'r').read().splitlines())

def none_to_0(x):
	if x is None:
		return 0
	return x

if __name__=='__main__':
	p=ArgumentParser()
	p.add_argument('--file',default='data/2017_pnw_post.txt')
	args=p.parse_args()
	data=parse_file(args.file)
	
	for a in data:
		qual_pts=none_to_0(a['event1'])+none_to_0(a['event2'])+a['team_age_points']
		print qual_pts,a['district_championship'],a['team']
