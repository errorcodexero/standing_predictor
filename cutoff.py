#!/usr/bin/env python

#Look at historical qualification data to figure out cutoffs.

import numpy as np
from argparse import ArgumentParser
from glob import glob
from predictor import flatten
from meta import print_lines
from sys import exit

def firsts(a):
	#any(a) => [[a]]->[a]
	return map(lambda x: x[0],a)


def seconds(a):
	#any(a) => [[a]]->[a]
	return map(lambda x: x[1],a)


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
		if s=='---': return None
		return int(i)

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

#TODO: Make it so that do the least squares for interesting percentiles
#so like 25th, 50th, 75th, 10th, 90th?
#or do this at every number of points?
#so odds that will make at least X points from each starting number of points
def calc_lines():
	f=flatten(map(parse_file,glob('data/20*_pnw_post.txt')))
	m=map(lambda x: (x['event1'],x['event2']),f)
	m=filter(lambda x: x[0] is not None and x[1] is not None,m)
	#print m[:10]
	#for e1_score in sorted(set(firsts(m))):
	#	print e1_score
	out={}
	for target_score in range(80):
		#print target_score,
		m1=map(lambda x: (x[0],x[1]>target_score),m)
		#print m1
		par=np.polyfit(firsts(m1), seconds(m1), 1, full=True)
		slope=par[0][0]
		intercept=par[0][1]
		def f(x): return slope*x+intercept
		#print '\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f'%(slope,intercept,f(4),f(20),f(30),f(50))
		out[target_score]=(slope,intercept)
	return out

#now just need to combine with a probability of the cutoff being a certain number of points.
#58 mean
#stddev of 2?
#58 .5
#57/59 .125 each
#56/60 1/16
#55/61 1/16

from scipy.stats import norm

def cutoff_odds():
	#print 'norm:',np.random.normal(58,5)

	out={}
	for i in range(40,80):
		out[i]=norm.pdf(i,58,5)
	#print_lines(sorted(out.iteritems()))
	return out

	raise 'nyi'
	return {
		61:1.0/16,
		60:1.0/16,
		59:1.0/8,
		58:.5,
		57:1.0/8,
		56:1.0/16,
		55:1.0/16
		}

def lines_demo():
	lines=calc_lines()
	
	for existing_points in range(78):
		print existing_points,
		total_px=0
		for cutoff_points,cutoff_px in cutoff_odds().iteritems():
			points_needed=cutoff_points-existing_points
			if points_needed<=0:
				px=1
			else:
				slope,intercept=lines[points_needed]
				px=slope*existing_points+intercept
				px=min(px,1)
				px=max(px,0)
			total_px+=px*cutoff_px
		print total_px

def worlds(data):
	#print_lines(data)
	#return 0
	for line in data:
		#print line
		total_points=none_to_0(line['event1'])+none_to_0(line['event2'])+none_to_0(line['district_championship'])+line['team_age_points']
		print '%s\t%s\t%s'%(total_points,line['frc_championship'],line['team'])

if __name__=='__main__':
	p=ArgumentParser()
	p.add_argument('--file',default='data/2017_pnw_post.txt')
	p.add_argument('--mean',action='store_true')
        p.add_argument('--worlds',action='store_true')
	args=p.parse_args()
	data=parse_file(args.file)

	if args.worlds:
		exit(worlds(data))

	if args.mean:
		for a in data:
			qual_pts=none_to_0(a['event1'])+none_to_0(a['event2'])+a['team_age_points']
			print qual_pts,a['district_championship'],a['team']

	lines_demo()
