#!/usr/bin/env python

from meta import actual_results,print_lines,predictions

def values(d):
    r=[]
    for k,v in d.iteritems():
        r.append(v)
    return set(r)

def read():
    #returns str(heading)->[int(team number)]
    lines=file('data/inflight.txt').read().splitlines()
    data={} #str(heading)->[int(team number)]
    current_heading=None
    for line in lines:
        sp=line.split()
        if len(sp)==0:
            continue
        def item():
            try:
                return int(sp[0])
            except:
                return line
        i=item()
        if type(i) is int:
            data[current_heading].append(i)
        else:
            current_heading=i
            data[current_heading]=[]
    return data


def contingency():
    via_in_flight=read()
    pred=predictions('results/2017_mar19_pnw.txt')
    #print_lines(pred.iteritems())
    #(label,prediction)->[team]
    def get_label(team):
        for label,teams in via_in_flight.iteritems():
            if team in teams:
                return label
    out={}
    for team,prediction in pred.iteritems():
        k=get_label(team),prediction
        if k not in out:
            out[k]=[]
        out[k].append(team)
    prediction_types=['in','~in','bubble','~out','out',"InFlight's label"]#list(values(pred))
    print '\t'.join(prediction_types)
    for label in list(via_in_flight)+[None]:
        def get(prediction):
            k=(label,prediction)
            if k in out:
                return len(out[k])
            return 0
        print '\t'.join(map(lambda x: str(get(x)),prediction_types)+[str(label)])

    print 'Specific teams:'
    for k,v in sorted(out.iteritems()):
        print k
        print '\t',sorted(v)

if __name__=='__main__':
    #for heading,teams in read().iteritems():
    #    print heading
    #    print teams
    contingency()
    
