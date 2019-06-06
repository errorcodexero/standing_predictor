#!/usr/bin/env python

from glob import glob
from BeautifulSoup import BeautifulSoup
from util import print_lines

def show(s):
    #iterable->void
    print_lines(list(enumerate(s)))


def get_team(data):
    #soup->int
    return int(data.contents[1].contents[0].split()[0])


def get_dcmp(data):
    #soup->bool
    s=data.contents[0].strip()
    if s=='---': return 0
    if 'See Team Page' in str(data):
        return 1
    try:
        int(s)
        return 1
    except:
        print 'str:',s
        print data
        raise 'nyi'


def parse_results(filename):
    #str->(str(district code),(Team->bool))
    district_code=filename.split('.')[0]
    soup=BeautifulSoup(open(filename,'r'))
    #print_lines(enumerate(soup.contents[2].contents[3].contents[9].contents[1].contents[3].contents[9].contents[3].contents))
    rows=soup.contents[2].contents[3].contents[9].contents[1].contents[3].contents[9].contents[3].contents
    by_team={}
    for row in rows:
        if row=='\n':
            continue
        team=get_team(row.contents[5])
        dcmp_pts=get_dcmp(row.contents[11])
        #print_lines(list(enumerate(row.contents)))
        #raise 'nyi'
        by_team[team]=dcmp_pts
    return district_code,by_team


def main():
    r=map(parse_results,glob("*html"))
    #print r
    for district,result in r:
        fd=open(district+'.csv','w')
        for item in result.iteritems():
            print>>fd,'%d,%d'%item


if __name__=='__main__':
    main()
