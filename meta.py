#!/usr/bin/env python

#open results file 
#open data file of who got in
#could how many went in what category

from predictor import parse_file
from sys import exit

#any(a) => [a] -> void
def print_lines(a):
	for elem in a: print elem

#any(k),any(v) => {k->v} -> void
def print_map(a):
	for key in sorted(a):
		print key,a[key]

#any(a),any(b) => (a->b)->[a]
def keys(m): return sorted(m)

#returns map from int(team number) to bool(in)
def actual_results(filename):
	official=parse_file(filename)
	team_results={}
	for entry in official:
		team_results[entry['team_number']]=entry['dcmp_qual']
	return team_results

#returns map from int(team number) to str(prediction)
def predictions(filename):
	#str->(str,int)
	def parse_line(s):
		sp=s.split('\t')
		return sp[1],int(sp[2].split()[0])

	m=map(parse_line,file(filename).read().splitlines()[4:])
	r={}
	for prediction,team in m:
		r[team]=prediction
	return r

def pnw_attendees():
	def parse_line(s):
		sp=s.split('\t')
		return {
			'team_number':int(sp[0]),
			'team_name':sp[1],
			'city':sp[2],
			'state':sp[3],
			'qualification_reason':sp[4]
			}
	return map(parse_line,open('data/2018_pnw_attendees.txt').read().splitlines())

def pnw_load_in():
	def parse_line(s):
		sp=s.split('\t')
		return {
			'team_number':int(sp[0]),
			'vehicle':sp[1],
			'time':sp[2]
			}
	return map(parse_line,open('data/2018_pnw_load_in.txt').read().splitlines())

def demo():
	#print_lines(pnw_attendees())
	#print_lines(pnw_load_in())
	a=set(map(lambda x: x['team_number'],pnw_attendees()))
	b=set(map(lambda x: x['team_number'],pnw_load_in()))
	assert a==b

	p1=pnw_attendees()

	p=parse_file('data/2018_pnw_rank.txt')
	#print_lines(p)
	for a in p:
		team_number=int(a['team'].split()[0])
		f=filter(lambda x: x['team_number']==team_number,p1)
		if len(f):
			q=f[0]['qualification_reason'][:7]
		else:
			q=None
		print '\t'.join(map(str,[a['rank'],a['total_points'],q,a['team']]))

def actual_2018():
	p=parse_file('data/2018_pnw_rank.txt')
	p1=pnw_attendees()
	out={}
	for a in p:
		team=a['team']
		team_number=int(team.split()[0])
		f=filter(lambda x: x['team_number']==team_number,p1)
		if len(f):
			v='in'
		else:
			v='out'
		out[team_number]=v
	#print out
	return out

def run(actual_file,prediction_file):
	#actual=actual_results(actual_file)
	actual=actual_2018()
	pred=predictions(prediction_file)

	#Sanity check that both lists have the same teams
	t1=keys(actual)
	t2=keys(pred)
	if t1!=t2:
		print set(t1)-set(t2)
		print set(t2)-set(t1)
	assert t1==t2

	result={}
	def add_result(predicted,actual):
		if not result.has_key(predicted):
			result[predicted]={'in':0,'out':0}
		result[predicted][actual]+=1

	for team in t1:
		add_result(pred[team],actual[team])

	#print_map(result)
	print '\t'.join(['Predicted','Actual In','Actual Out'])
	for pred in ['bubble','in','out','~in','~out']:
		d=result[pred]
		print '\t\t'.join(map(str,[pred,d['in'],d['out']]))

if __name__=='__main__':
	from optparse import OptionParser
	p=OptionParser()
	p.add_option('--actual',default='data/2016_apr4_pnw.txt')
	p.add_option('--predicted',default='results/2016_mar19_pnw.txt')
	p.add_option('--demo',action='store_true')
	options,args=p.parse_args()
	assert len(args)==0
	if options.demo:
		exit(demo())
	run(options.actual,options.predicted)
