#!/usr/bin/env python

from util import print_lines,keys,group,print_r,mean,seconds,skip_while,pad_to_width
from math import sqrt
from glob import glob
from BeautifulSoup import BeautifulSoup
from official import parse


def map_keys(f,m):
    r={}
    for k,v in m.iteritems():
        r[k]=f(v)
    return r


def parse_row(d):
    #soup->(int(team),float(px))
    px=float(d.contents[1].contents[0])
    team=int(d.contents[2].contents[0].contents[0])
    return team,px


def parse_file(filename):
    #str->(district name(str),(Team->float))
    soup=BeautifulSoup(open(filename,'r'))
    #print_lines(list(enumerate(soup.contents[0].contents[1].contents[11].contents[1:])))
    r1=skip_while(lambda x: 'Team Probabilities' not in str(x),soup.contents[0].contents[1].contents)
    rows=r1[1:][0].contents[1:]
    #print rows
    return filename.split('/')[-1][4:].split('.')[0],dict(map(parse_row,rows))


def to_official_name(s):
    #str->str
    if s=='fnc': return 'nc'
    return s


def bucket(x):
    #float->(int,str)
    d=x[0]
    assert d>=0
    assert d<=1
    if d==0:
        return (0,'x=0')
    if d==1:
        return (100,'x=100')
    low=int(d*10)*10
    ctr=low+5
    if low==0:
        return (ctr,'0<x<10')
    return (ctr,'%d<=x<%d'%(low,low+10))


def show_calibration(results):
    g=group(bucket,results)
    dec=map_keys(lambda data: (mean(seconds(data)),len(data)),g)
    print 'Predicted Observed Samples'
    for a,b in sorted(dec.iteritems()):
        print pad_to_width(9,a[1]),pad_to_width(8,str(round(b[0],2))),b[1]
    residuals=map(lambda x: x[0]-x[1],results)
    print 'mad:',mean(map(abs,residuals))
    print 'rms:',sqrt(mean(map(lambda x: x*x,residuals)))


def parse_txt_file(filename):
    rest=skip_while(lambda x: 'Team #' not in x,open(filename,'r').read().splitlines())[1:]
    def parse_line(s):
        sp=s.split()
        team=int(sp[0])
        px=float(sp[1])
        return team,px
    return filename.split('/')[-1].split('.')[0],dict(map(parse_line,rest))


def check_week(week):
    verbose=0
    m=map(parse_file,glob(week+'/*.html'))
    m.extend(map(parse_txt_file,glob(week+'/*.txt')))
    all1=[]
    for district,predicted in m:
        if verbose:
            print district
        #print predicted
        d2,actual=parse.parse_results('official/'+to_official_name(district)+'.html')
        teams=set(keys(predicted))
        actual_teams=set(keys(actual))

        missing_teams=teams-actual_teams
        not_predicted=actual_teams-teams
        if verbose:
            print 'Extra predictions:',missing_teams
            print 'Not predicted:',not_predicted
        assert len(not_predicted)==0

        #This happens for teams that didn't end up showing up to events.
        for x in missing_teams:
            actual[x]=0

        #for team in teams:
        #    print team,predicted[team],actual[team]
        results=map(lambda x: (predicted[x],actual[x]),teams)
        all1.extend(results)
    show_calibration(all1)
    return all1

def main():
    overall=[]
    for week in sorted(glob('mar*')):
        print week
        overall.extend(check_week(week))
        print
    print 'Overall:'
    show_calibration(overall)

if __name__=='__main__':
    main()
