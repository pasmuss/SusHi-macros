#!/usr/bin/python

from ROOT import TTree,TFile,TH3F
from array import array
import math

def TranslateCosB_To_sinA(cos_bins):
    """Function to trasfer cos(beta-alpha) binning in conv. B to sin(beta-alpha) in conv. A.

    convention B: 0 < beta-alpha < Pi
    convention A: -90 < beta-alpha < 90
    """
    sin_beta_alpha_A = []
    for p in cos_bins:
        #calculate the sin
        s_beta_alpha_B = math.sqrt(1 - p**2)
        #translate convention B to A
        s_beta_alpha_A = s_beta_alpha_B
        if p < 0:
            s_beta_alpha_A *= -1
        s_beta_alpha_A = round(s_beta_alpha_A,5)
        sin_beta_alpha_A.append(s_beta_alpha_A)
    return sin_beta_alpha_A

def MakeBins(l):
#    print "l",l
    L = array('f')
    L.append(l[0] - (l[1]-l[0])/2)
#    print "L", l[0] - (l[1]-l[0])/2
    #make bin boundaries
    for i in range(len(l)):
#        print "Checking element of l", i
#        print "l[i]", l[i], 'L ', l[i]+(l[i]-L[-1]), 'L[-1]', L[-1]
        L.append(l[i]+(l[i]-L[-1]))
    #check if bin boundaries are in increasing order
    for i in range(len(L)-1):
#        print "Checking element of L", i
#        print "L[i]", L[i], "L[i+1]", L[i+1], "len(L)", len(L)
        if L[i+1]-L[i] <= 0:
            raise RuntimeError("Check bin boundaries!!!")
    return L

#tanBetas
tanBetaLowRange = [0.5,2.]
tanBetaLowStep  = 0.05
tanBetasLow = [ round(tanBetaLowRange[0]+tanBetaLowStep*i,3) for i in range(int((tanBetaLowRange[1]-tanBetaLowRange[0])/tanBetaLowStep)) ]
tanBetaRange = [2.,100.]
tanBetaStep = 0.5
tanBetas = [ tanBetaRange[0]+tanBetaStep*i for i in range(int((tanBetaRange[1]-tanBetaRange[0])/tanBetaStep+1)) ]
tanBetas = tanBetasLow + tanBetas
#tanBetas = [10.0,20.0,30.0,40.0,50.0,60.0]

#H masses (since type of boson is defined elsewhere: mH and mA equally used here)
mH = [125,130,140,160,180,200,250,300,350,400,450,500,600,700,800,900,1000,1100,1200,1300,1400,1600,1800,2000]
#mH = [300,350,400,500,600,700,900,1100]
#h masses
mh = [125.]
#sin(beta - alpha)
cosB_A_Range = [-1.,1.]
cosB_A_step = 0.02
cosB_As = [cosB_A_Range[0] + cosB_A_step* i for i in range(int((cosB_A_Range[1] - cosB_A_Range[0])/cosB_A_step + 1))]
#cosB_As = [-0.8,-0.3,0.0,0.5]
sinB_As = TranslateCosB_To_sinA(cosB_As)
#sinB_As = [-0.8,-0.3,0.0,0.5]
print sinB_As

#type_boson = 'type1_mH'
type_bosons = ['type1_mH','type2_mH','type3_mH','type4_mH','type1_mA','type2_mA','type3_mA','type4_mA','type1_mh','type2_mh','type3_mh','type4_mh']

for type_boson in type_bosons:
    inputTxt = '/nfs/dust/cms/user/asmusspa/public/CMSSW_9_2_15/src/Analysis/MssmHbb/SusHi/FullRun_100PerJob_AllTypesAndBosons/txtFiles/' + type_boson + '.txt'
    print inputTxt

# names of the column
    lines = [ line.split() for line in open(inputTxt, 'r')]
    histNames = lines[0]
    print histNames
    mH_bin = MakeBins(mH)
    tanBetas_bin = MakeBins(tanBetas)
#sinB_As_bin = MakeBins(sinB_As)
    cosB_As_bin = MakeBins(cosB_As)

    print 'TAN BETAs:'
    print(tanBetas)
    print 'cos(beta-alpha)s:'
    print sinB_As
#print cosB_As

# print sinB_As_bin
#print "len mH bin: ",str(len(mH_bin)-1)
#print "mH bin: ", str(mH_bin)
#print "len tan beta bin: " , str(len(tanBetas_bin)-1)
#print "tan beta bin: " , str(tanBetas_bin)
#print "len cos bins: " , str(len(cosB_As_bin)-1)
#print "cos bins: " , str(cosB_As_bin)
#print "len hists: " , str(len(histNames))
    hists = [ TH3F(histNames[i], ";mH;tan(#beta);cos(#beta-#alpha)", len(mH_bin)-1, mH_bin, len(tanBetas_bin)-1, tanBetas_bin, len(cosB_As_bin)-1, cosB_As_bin) for i in range(5, len(histNames))]

# Ouptut file
    output_name = '/nfs/dust/cms/user/asmusspa/public/CMSSW_9_2_15/src/Analysis/MssmHbb/SusHi/FullRun_100PerJob_AllTypesAndBosons/rootFiles/Histograms3D_' + type_boson + '.root'
    f = TFile(output_name,'recreate')
    for i in range(1, len(lines)):
        for j in range(5, len(histNames)):
            massH   = float(lines[i][3])
            #print massH
            tanBeta = float(lines[i][0])
            #print tanBeta
            sinB_A  = float(lines[i][1])
        #Round
            sinB_A = round(sinB_A,5)
            #print sinB_A
        #special index settings to have cos(B-A) = 1 in the histogram
            sinB_A_index = sinB_As.index(sinB_A)
            #print sinB_A_index
            if lines[i][1] == '-0.00000000E+00': sinB_A_index = 400
    
#         if massH == 300 and tanBeta == 20 and sinB_A == 0.9: print 'tanBeta: %0.2f, sin(beta-alpha): %0.3f, mH = %f, val = %0.3f' % (tanBeta, sinB_A, massH,float(lines[i][j])) + ' hist name: ' +  histNames[j]

#         print sinB_A
        # if [st not in histNames[j] for st in ('unitarity','perturbativity','stability')]:
#        print('mH = %f , tanB = %f , sinB_A = %f , VAL = %f ' % ((massH),(tanBeta),(sinB_A), float(lines[i][j])))
#        print str(hists[j-5])
#        print str(mH.index(massH)), mH
#        print str(tanBetas.index(tanBeta)), tanBetas
#        print sinB_A_index, sinB_As, sinB_A
            hists[j-5].SetBinContent(mH.index(massH) + 1, tanBetas.index(tanBeta) + 1, sinB_A_index + 1, float(lines[i][j]))
#        print "Bin content hist", hists[j-5]," (", mH.index(massH) + 1, "/" , massH, "," , tanBetas.index(tanBeta) + 1, "/", tanBeta, "," , sinB_A_index + 1, "/", sinB_A, "): ", float(lines[i][j])

            if i == len(lines)-1:
                hists[j-5].Write()
# if histNames[j] != 'unitarity' and histNames[j] != 'perturbativity' and histNames[j] != 'stability':
    f.Close()
