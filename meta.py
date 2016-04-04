#!/usr/bin/env python

#open results file 
#open data file of who got in
#could how many went in what category

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
	from predictor import parse_file
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

def run(actual_file,prediction_file):
	actual=actual_results(actual_file)
	pred=predictions(prediction_file)

	#Sanity check that both lists have the same teams
	t1=keys(actual)
	t2=keys(pred)
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
	options,args=p.parse_args()
	assert len(args)==0
	run(options.actual,options.predicted)
