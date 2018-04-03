#!/usr/bin/env python

from cutoff import parse_file,none_to_0,seconds

def rp_parse_line(s):
	#str->(int,int)
	sp=s.split()
	assert len(sp)==2
	return int(sp[0]),int(sp[1])


def rp_parse_file():
	#returns [(int,int)]
	return map(rp_parse_line,open('data/pts_64.txt','r').read().splitlines())


def rank_to_get_points(points):
	#int->either int or str
	r=rp_parse_file()
	point_values=seconds(r)
	#if points<=min(point_values):
	#	return 'any'
	for x in reversed(r):
		rank=x[0]
		if rank<=8:
			points_here=x[1]+(17-rank)
		else:
			points_here=x[1]
		if points<=points_here:
			return rank


if __name__=='__main__':
	cutoff=122
	p=filter(lambda x: x['district_championship'],parse_file('data/2018_mar31_pnw.txt'))
	for a in p:
		total_points=none_to_0(a['event1'])+none_to_0(a['event2'])+a['team_age_points']
		normal_points_left=(cutoff-total_points)/3.0
		r=rank_to_get_points(normal_points_left)
		#print '%s\t%s\t%s'%(r,normal_points_left,a['team'])
		print '%s\t%s'%(r,a['team'])


