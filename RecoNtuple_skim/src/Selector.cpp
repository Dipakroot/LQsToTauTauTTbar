#include"../interface/Selector.h"

// double dR(double eta1, double phi1, double eta2, double phi2){
//     double dphi = phi2 - phi1;
//     double deta = eta2 - eta1;
//     static const double pi = TMath::Pi();
//     dphi = TMath::Abs( TMath::Abs(dphi) - pi ) - pi;
//     return TMath::Sqrt( dphi*dphi + deta*deta );
// }

TRandom* generator = new TRandom3(0);

Selector::Selector(){

    year = "2016";

    // jets
    jet_Pt_cut = 30;
    jet_Eta_cut = 2.4;

    printEvent = -1;

    looseJetID = false;
    veto_lep_jet_dR = 0.4; // remove jets with a lepton closer than this cut level
    JERsystLevel  = 1;
    JECsystLevel  = 1;
    elesmearLevel = 1;
    elescaleLevel = 1;
    useDeepCSVbTag = false;
    //https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation80XReReco
    // CSVv2M
    btag_cut = 0.8484;  
    // DeepCSV
    btag_cut_DeepCSV = 0.6324;  


    // whether to invert lepton requirements for 
    QCDselect = false;

    // electrons
    ele_Pt_cut = 35.0;
    ele_Eta_cut = 2.4;
    ele_PtLoose_cut = 15.0;
    ele_EtaLoose_cut = 2.4;

    // muons
    mu_Pt_cut = 30;
    mu_Eta_tight = 2.4;
    mu_RelIso_tight = 0.15;

    mu_PtLoose_cut = 15.0;
    mu_Eta_loose = 2.4;
    mu_RelIso_loose = 0.25;
	
    mu_Iso_invert = false;
    smearJetPt = true;
    smearEle = true;
    scaleEle = true;
    
    //Added by Dipak
    // taus
    tau_Pt_cut = 30;
    tau_Eta_cut = 2.3;
    
}

void Selector::init_JER(std::string inputPrefix){

    jetResolution = new JME::JetResolution((inputPrefix+"_MC_PtResolution_AK4PFchs.txt").c_str());
    jetResolutionScaleFactor = new JME::JetResolutionScaleFactor((inputPrefix+"_MC_SF_AK4PFchs.txt").c_str());

    //jetParam = new JME::JetParameters();

    // cout << "INIT JER" << endl;
    // JERParam = new JetCorrectorParameters((inputPrefix+"_MC_PtResolution_AK4PFchs.txt").c_str());
    // cout << "INIT JER" << endl;
    // std::vector<JetCorrectorParameters> vPar;
    // cout << "INIT JER" << endl;
    // vPar.push_back(*JERParam);
    // cout << "INIT JER" << endl;
    // JetCorrector = new FactorizedJetCorrector(vPar);
    // cout << "INIT JER" << endl;

    // cout << JetCorrector << endl;
}

void Selector::process_objects(EventTree* inp_tree){
    tree = inp_tree;
    //    TFile *f1 =TFile::Open("../../Skim_NanoAOD/output_2018_tauAdd.root","READONLY");
    //TTree *tree = (TTree*)f1->Get("RecoNtuple_Skim");
    clear_vectors();
    //cout << "before selector muons" << endl;
    filter_muons();

    //cout << "before selector electrons" << endl;
    filter_electrons();

    //cout << "before selector jets" << endl;
    filter_jets();

    // added by Dipak
    //    cout << "before selector taus" << endl;                                                                                                                                   
    filter_taus();

}

void Selector::clear_vectors(){
    Electrons.clear();
    ElectronsLoose.clear();
    ElectronsMedium.clear();
    ElectronsNoIso.clear();
    Muons.clear();
    MuonsLoose.clear();
    MuonsNoIso.clear();
    Jets.clear();
    FwdJets.clear();
    bJets.clear();
	
    MuRelIso_corr.clear();

    //Added by Dipak
    TausLoose.clear();
    TausMedium.clear();
    TausTight.clear();
    Taus.clear();
}

void Selector::filter_electrons(){
    if (tree->event_==printEvent){
	cout << "Found Event, Starting Electrons" << endl;
	cout << " nEle=" << tree->nEle_ << endl;
    }

    for(int eleInd = 0; eleInd < tree->nEle_; ++eleInd){

        double eta = tree->eleEta_[eleInd];
        double absEta = TMath::Abs(eta);
        double SCeta = eta + tree->eleDeltaEtaSC_[eleInd];
        double absSCEta = TMath::Abs(SCeta);

        double pt = tree->elePt_[eleInd];
        
        // // EA subtraction
        double PFrelIso_corr = tree->elePFRelIso_[eleInd];
        
        uint eleID = tree->eleIDcutbased_[eleInd];
        bool passVetoID   = eleID >= 1;
        bool passLooseID  = eleID >= 2;
        bool passMediumID = eleID >= 3;
        bool passTightID  = eleID >= 4;
        
        // make sure it doesn't fall within the gap
        bool passEtaEBEEGap = (absSCEta < 1.4442) || (absSCEta > 1.566);
        

        // D0 and Dz cuts are different for barrel and endcap
        bool passD0 = ((absEta < 1.479 && abs(tree->eleD0_[eleInd]) < 0.05) ||
                       (absEta > 1.479 && abs(tree->eleD0_[eleInd]) < 0.1));
        bool passDz = ((absEta < 1.479 && abs(tree->eleDz_[eleInd]) < 0.1) ||
                       (absEta > 1.479 && abs(tree->eleDz_[eleInd]) < 0.2));
        
        

        double EleSmear = 1.;

        /////////NEEDS TO BE REIMPLEMENTED

        // if (!tree->isData_ && elesmearLevel==1) {EleSmear = generator->Gaus(1,(tree->eleResol_rho_up_[eleInd]+tree->eleResol_rho_dn_[eleInd])/2.);}
        // if (!tree->isData_ && elesmearLevel==0) {EleSmear = generator->Gaus(1,tree->eleResol_rho_dn_[eleInd]);}
        // if (!tree->isData_ && elesmearLevel==2) {EleSmear = generator->Gaus(1,tree->eleResol_rho_up_[eleInd]);}
        // if (pt<10.){
        //   smearEle= false;
        // }
        // if (smearEle){
        //   pt = pt*EleSmear;
        //   en = EleSmear*en;
        // }
        // tree->elePt_[eleInd] = pt;
        // tree->eleEn_[eleInd]= en;   
        // double EleScale = 1.;
        // double nom_scale =  (float(tree->eleScale_stat_up_[eleInd]+tree->eleScale_stat_dn_[eleInd])/2.);
        // if (tree->isData_ && elescaleLevel==1) {EleScale = ((tree->eleScale_stat_up_[eleInd]+tree->eleScale_stat_dn_[eleInd])/2.);}
        // if (!tree->isData_ && elescaleLevel==2){EleScale = 1.+sqrt(pow((1-tree->eleScale_syst_up_[eleInd]),2)+pow((1-tree->eleScale_stat_up_[eleInd]),2)+pow((1-tree->eleScale_gain_up_[eleInd]),2));}
        // if (!tree->isData_ && elescaleLevel==0){EleScale = 1.-(sqrt(pow((1-tree->eleScale_syst_dn_[eleInd]),2)+pow(1-(tree->eleScale_stat_dn_[eleInd]),2)+pow((1-tree->eleScale_gain_dn_[eleInd]),2)));}
        // if (scaleEle){
        //   //	std::cout<<tree->eleScale_syst_dn_[eleInd]<<   tree->eleScale_stat_dn_[eleInd]<<   tree->eleScale_syst_dn_[eleInd]<<std::endl;
        //   //		std::cout<<"nominal is:"<< nom_scale<<std::endl;
        //   //	std::cout << "stat: "<<EleScale <<std::endl;
        //   pt = pt*EleScale;
        //   en = EleScale*en;
        // }       
        // tree->elePt_[eleInd] = pt;
        // tree->eleEn_[eleInd]= en;
        

	// upper limit on the QCD iso of the vetoID value
	bool passTightID_noIso = passEleID(eleInd, 4,false) && passVetoID;
	
        if (QCDselect){
            Float_t isoEBcut = 0.0287 + 0.506/tree->elePt_[eleInd];
            Float_t isoEECut = 0.0445 + 0.963/tree->elePt_[eleInd];
            passTightID = false;
            passTightID = (passTightID_noIso && 
			   PFrelIso_corr > (absSCEta < 1.479 ? isoEBcut : isoEECut) );
        }


	bool passVetoIDNoIso = passEleID(eleInd, 1,false); // Ignore iso requirements

	float vetoIsoEBcut = 0.198 + 0.506/tree->elePt_[eleInd];
	float vetoIsoEECut = 0.203 + 0.963/tree->elePt_[eleInd];

	//QCD CR now applies veto iso as upper limit, so Veto leptons are the same as they would be otherwise

	// // if using QCD cr, just use cut without iso requirement
	// if(QCDselect){
	//     passVetoID = passVetoIDNoIso;
	// }

        bool eleSel = (passEtaEBEEGap && 
                       absEta <= ele_Eta_cut &&
                       pt >= ele_Pt_cut &&
                       passTightID &&
                       passD0 &&
                       passDz);

        bool looseSel = ( passEtaEBEEGap && 
			  absEta <= ele_EtaLoose_cut &&
			  pt >= ele_PtLoose_cut &&
			  passVetoID &&
			  passD0 &&
			  passDz);
        
	bool eleSel_noIso = (passEtaEBEEGap && 
			     absEta <= ele_Eta_cut &&
			     pt >= ele_Pt_cut &&
			     passTightID_noIso &&
			     passD0 &&
			     passDz);
	
        
	if (tree->event_==printEvent){
	    cout << "-- " << eleInd << " eleSel=" <<  eleSel << " looseSel=" <<  looseSel << " pt="<<pt<< " eta="<<eta<< " phi="<<tree->elePhi_[eleInd]<< " eleID="<<eleID << " passD0="<<passD0<< "("<<tree->eleD0_[eleInd]<<") passDz="<<passDz<< "("<<tree->eleDz_[eleInd]<<")"<< endl;
	    cout << "            ";
	    std::cout << std::setbase(8);
	    cout << "            idBits="<<tree->eleVidWPBitmap_[eleInd] << endl;
	    std::cout << std::setbase(10);
	    
	} 
        
	
        if( eleSel ){
            Electrons.push_back(eleInd);
        }
        else if( looseSel ){ 
            ElectronsLoose.push_back(eleInd);
        }
        if( eleSel_noIso ){
            ElectronsNoIso.push_back(eleInd);
        }
    }

    if (tree->event_==printEvent){
	if (year=="2016"){
	    cout << "            idBits :   MinPtCut, GsfEleSCEtaMultiRangeCut, GsfEleDEtaInSeedCut, GsfEleDPhiInCut, GsfEleFull5x5SigmaIEtaIEtaCut, GsfEleHadronicOverEMCut, GsfEleEInverseMinusPInverseCut, GsfEleEffAreaPFIsoCut, GsfEleConversionVetoCut, GsfEleMissingHitsCut" << endl;
	} else {
	    cout << "            idBits :   MinPtCut, GsfEleSCEtaMultiRangeCut, GsfEleDEtaInSeedCut, GsfEleDPhiInCut, GsfEleFull5x5SigmaIEtaIEtaCut, GsfEleHadronicOverEMEnergyScaledCut, GsfEleEInverseMinusPInverseCut, GsfEleRelPFIsoScaledCut, GsfEleConversionVetoCut, GsfEleMissingHitsCut" << endl;
	}
    }
    
}

void Selector::filter_muons(){
    if (tree->event_==printEvent){
	cout << "Found Event, Starting Muons" << endl;
	cout << " nMu=" << tree->nMuon_ << endl;
    }
    for(int muInd = 0; muInd < tree->nMuon_; ++muInd){

	double eta = tree->muEta_[muInd];
	double pt = tree->muPt_[muInd];

	double PFrelIso_corr = tree->muPFRelIso_[muInd];

	bool looseMuonID = tree->muIsPFMuon_[muInd] && (tree->muIsTracker_[muInd] || tree->muIsGlobal_[muInd]);
	bool mediumMuonID = tree->muMediumId_[muInd];
	bool tightMuonID = tree->muTightId_[muInd];

	bool passTight = (pt >= mu_Pt_cut &&
			  TMath::Abs(eta) <= mu_Eta_tight &&
			  tightMuonID &&
			  (!QCDselect ? (PFrelIso_corr < mu_RelIso_tight): PFrelIso_corr > mu_RelIso_tight)
			  && (PFrelIso_corr < mu_RelIso_loose) //for QCD , upper limit of iso of the loose cut
			  );

	bool passLoose = (pt >= mu_PtLoose_cut &&
			  TMath::Abs(eta) <= mu_Eta_loose &&
			  looseMuonID &&
			  (PFrelIso_corr < mu_RelIso_loose)
			  //(!QCDselect ? (PFrelIso_corr < mu_RelIso_loose): PFrelIso_corr > mu_RelIso_loose) 
			  );
	// if (QCDselect){ //ignoring Iso cut in  QCDCR
	//     bool passLoose = (pt >= mu_PtLoose_cut &&
	// 		      TMath::Abs(eta) <= mu_Eta_loose &&
	// 		      looseMuonID 
	// 		      );
	// }
	bool passTight_noIso = (pt >= mu_Pt_cut &&
				TMath::Abs(eta) <= mu_Eta_tight &&
				tightMuonID
				);
	
	if (tree->event_==printEvent){
	    cout << "-- " << muInd << " passTight="<<passTight<< " passLoose="<<passLoose << " pt="<<pt<< " eta="<<eta<< " phi="<<tree->muPhi_[muInd]<< " tightID="<<tightMuonID<< " looseID="<<looseMuonID << " pfRelIso="<<PFrelIso_corr << endl;
	} 
	
	if(passTight){
	    Muons.push_back(muInd);
	}
	else if (passLoose){
	    MuonsLoose.push_back(muInd);
	}
	if(passTight_noIso){
	    MuonsNoIso.push_back(muInd);
	}
    }
}


//Added by Dipak

void Selector::filter_taus(){
  if (tree->event_==printEvent){
    cout << "Found Event, Starting Taus" << endl;
    cout << " nTau=" << tree->nTau_ << endl;
  }
  for(int tauInd = 0; tauInd < tree->nTau_; ++tauInd){

    double eta = tree->tauEta_[tauInd];
    double pt = tree->tauPt_[tauInd];
    double dz = tree->tauDz_[tauInd];
    Int_t  charge = tree->tauCharge_[tauInd];
    Int_t decayMode = tree->tauDecayMode_[tauInd];

    int tauIdDeepTau2017v2p1VSe = tree->tauIdDeepTau2017v2p1VSe_[tauInd];
    int tauIdDeepTau2017v2p1VSjet = tree->tauIdDeepTau2017v2p1VSjet_[tauInd];
    int tauIdDeepTau2017v2p1VSmu = tree->tauIdDeepTau2017v2p1VSmu_[tauInd];
    
    
    bool hadronicTau = (decayMode ==0 || decayMode==1 ||decayMode==2 ||decayMode==5 ||decayMode==6 ||decayMode== 10 ||decayMode== 11) ; 
      
    bool generalTau =( pt >= tau_Pt_cut && 
			TMath::Abs(eta) <= tau_Eta_cut &&
			dz <0.2 &&
			TMath::Abs(charge) ==1
			);
    bool looseTauID =( tauIdDeepTau2017v2p1VSe >= 1 &&
		       tauIdDeepTau2017v2p1VSjet >=1 && 
		       tauIdDeepTau2017v2p1VSmu >= 1 
                          );
    bool looseTauIDTest =( tauIdDeepTau2017v2p1VSjet >=1 &&
                       tauIdDeepTau2017v2p1VSmu >= 1
		       );


    bool mediumTauID =( tauIdDeepTau2017v2p1VSe > 16 &&
		       tauIdDeepTau2017v2p1VSjet >16 &&
		       tauIdDeepTau2017v2p1VSmu> 4
		       );

    bool tightTauID =( tauIdDeepTau2017v2p1VSe > 128 &&
		       tauIdDeepTau2017v2p1VSjet >128 &&
		       tauIdDeepTau2017v2p1VSmu> 8
		       );

    bool passLoose = ( hadronicTau && generalTau && looseTauID );
    bool passLooseTest = (hadronicTau && generalTau && looseTauIDTest );
    bool passMedium = (hadronicTau && generalTau && mediumTauID);

    bool passTight = ( hadronicTau && generalTau && tightTauID );

    // cout << "This is inside tau filter" << endl;

   

    if (tree->event_==printEvent){
      cout << "-- " << tauInd <<" passTight="<<passTight<< "passMedium" << passMedium <<" passLoose="<<passLoose <<" pt="<<pt<< " eta="<<eta<< " phi="<<tree->tauPhi_[tauInd]<< " tightID="<<tightTauID<< "mediumID" << mediumTauID<<" looseID="<<looseTauID << endl;
    }



    if(passLoose)
      Taus.push_back(tauInd); //test
    
    if(passLoose)
      TausLoose.push_back(tauInd);                                                                                                                                       
    if(passMedium)
      TausMedium.push_back(tauInd); 
    
    if(passTight)
      TausTight.push_back(tauInd);   
    
    
  }
}




void Selector::filter_jets(){
  //    TLorentzVector tMET;
    if (tree->event_==printEvent){
    	cout << "Found Event Staring Jets" << endl;
    }

    for(int jetInd = 0; jetInd < tree->nJet_; ++jetInd){
        double pt = tree->jetPt_[jetInd];
        double eta = tree->jetEta_[jetInd];
        double phi = tree->jetPhi_[jetInd];

	//tight ID for 2016 (bit 0), tightLeptVeto for 2017 (bit 1)
	int jetID_cutBit = 1;
	if (year=="2016"){ jetID_cutBit = 0; }
	
        bool jetID_pass = (tree->jetID_[jetInd]>>0 & 1 && looseJetID) || (tree->jetID_[jetInd]>>jetID_cutBit & 1);
        
	double jetSF = 1.;

        double resolution = 0.;
	if (!tree->isData_){
            jetParam.setJetEta(tree->jetEta_[jetInd]);
            jetParam.setJetPt(tree->jetPt_[jetInd]);
            jetParam.setJetArea(tree->jetArea_[jetInd]);
            jetParam.setRho(tree->rho_);
            resolution = jetResolution->getResolution(jetParam);
          
	    if (JERsystLevel==1) jetSF = jetResolutionScaleFactor->getScaleFactor(jetParam,Variation::NOMINAL);
	    if (JERsystLevel==0) jetSF = jetResolutionScaleFactor->getScaleFactor(jetParam,Variation::DOWN);
	    if (JERsystLevel==2) jetSF = jetResolutionScaleFactor->getScaleFactor(jetParam,Variation::UP);

	    double jetSmear = 1;

	    int genIdx = tree->jetGenJetIdx_[jetInd];
	    if ( (genIdx>-1) && (genIdx < tree->nGenJet_)){
		double genJetPt = tree->GenJet_pt_[genIdx];
		jetSmear = 1. + (jetSF - 1.) * (pt - genJetPt)/pt;
	    }else{
		jetSmear = 1 + generator->Gaus(0, resolution) * sqrt( max(jetSF*jetSF - 1, 0.) );
	    }

	    if (tree->event_==printEvent){
		cout << "DoJetSmear: " << smearJetPt << endl;
		cout << "GenIdx: "<< genIdx << endl;
		cout << "jetSF: "<< jetSF << endl;
		cout << "JetSmear: "<<jetSmear << endl;
	    }

	    if (smearJetPt){
		pt = pt*jetSmear;
		tree->jetPt_[jetInd] = pt;
	    }
	}

        bool passDR_lep_jet = true;

        //loop over selected electrons
        for(std::vector<int>::const_iterator eleInd = Electrons.begin(); eleInd != Electrons.end(); eleInd++) {
	    if (dR(eta, phi, tree->eleEta_[*eleInd], tree->elePhi_[*eleInd]) < veto_lep_jet_dR) passDR_lep_jet = false;
        }

        //loop over selected muons
        for(std::vector<int>::const_iterator muInd = Muons.begin(); muInd != Muons.end(); muInd++) {
          if (dR(eta, phi, tree->muEta_[*muInd], tree->muPhi_[*muInd]) < veto_lep_jet_dR) passDR_lep_jet = false;
        }
	// added by Dipak
	//loop over selected taus
	for(std::vector<int>::const_iterator tauInd = Taus.begin(); tauInd != Taus.end(); tauInd++){
	  if (dR(eta, phi, tree->tauEta_[*tauInd], tree->tauPhi_[*tauInd]) < veto_lep_jet_dR) passDR_lep_jet = false;
	}

        bool jetPresel = (pt >= jet_Pt_cut &&
                          TMath::Abs(eta) <= jet_Eta_cut &&
                          jetID_pass &&
                          passDR_lep_jet
                          );

	if (tree->event_==printEvent){
	    cout << "   pt=" << pt << "  eta=" << eta << " phi=" << phi << "  jetID=" << jetID_pass << endl;
	    cout << "         presel=" << jetPresel << endl;
	    cout << "              pt=" << (pt >= jet_Pt_cut) <<endl;
	    cout << "              eta=" << (TMath::Abs(eta) <= jet_Eta_cut) <<endl;
	    cout << "              jetID=" << jetID_pass <<endl;
	    cout << "              dRLep=" << passDR_lep_jet <<endl;
	    cout << "              btag="<<(tree->jetBtagDeepB_[jetInd] > btag_cut_DeepCSV) << endl; 

	}

        
        bool fwdjetPresel = (pt>= jet_Pt_cut && jetID_pass && TMath::Abs(eta)<=3.0 && TMath::Abs(eta)>2.5 &&
                             passDR_lep_jet
                             );
        
        if(fwdjetPresel){
            FwdJets.push_back(jetInd);
        }
        
        
        if( jetPresel){
            Jets.push_back(jetInd);
	    jet_resolution.push_back(resolution);
            if (!useDeepCSVbTag){
                if( tree->jetBtagCSVV2_[jetInd] > btag_cut){
		    bJets.push_back(jetInd);
		    jet_isTagged.push_back(true);
		} else {
		    jet_isTagged.push_back(false);
		}
            } else {
                if( tree->jetBtagDeepB_[jetInd] > btag_cut_DeepCSV){
		    bJets.push_back(jetInd);
		    jet_isTagged.push_back(true);
		} else {
		    jet_isTagged.push_back(false);
		}
            }				
        }
    }
    
    // // Update the MET for JEC changes
    // if (JECsystLevel==0 || JECsystLevel==2){
    // 	tree->pfMET_ = float(tMET.Pt());
    // 	tree->pfMETPhi_ = float(tMET.Phi());
    // }
}


bool Selector::passEleID(int eleInd, int cutVal, bool doRelisoCut){

    Int_t WPcutBits = tree->eleVidWPBitmap_[eleInd];

    int nBits = 3;

    bool MinPtCut                            = (WPcutBits>>(0*nBits) & 7) >= cutVal;
    bool GsfEleSCEtaMultiRangeCut            = (WPcutBits>>(1*nBits) & 7) >= cutVal;
    bool GsfEleDEtaInSeedCut                 = (WPcutBits>>(2*nBits) & 7) >= cutVal;
    bool GsfEleDPhiInCut                     = (WPcutBits>>(3*nBits) & 7) >= cutVal;
    bool GsfEleFull5x5SigmaIEtaIEtaCut       = (WPcutBits>>(4*nBits) & 7) >= cutVal;
    bool GsfEleHadronicOverEMEnergyScaledCut = (WPcutBits>>(5*nBits) & 7) >= cutVal;
    bool GsfEleEInverseMinusPInverseCut      = (WPcutBits>>(6*nBits) & 7) >= cutVal;
    bool GsfEleEffAreaPFIsoCut               = (WPcutBits>>(7*nBits) & 7) >= cutVal;
    bool GsfEleConversionVetoCut             = (WPcutBits>>(8*nBits) & 7) >= cutVal;
    bool GsfEleMissingHitsCut                = (WPcutBits>>(9*nBits) & 7) >= cutVal;

    bool passID = (MinPtCut
		   && GsfEleSCEtaMultiRangeCut
		   && GsfEleDEtaInSeedCut
		   && GsfEleDPhiInCut
		   && GsfEleFull5x5SigmaIEtaIEtaCut
		   && GsfEleHadronicOverEMEnergyScaledCut
		   && GsfEleEInverseMinusPInverseCut
		   && (GsfEleEffAreaPFIsoCut || !doRelisoCut)
		   && GsfEleConversionVetoCut
		   && GsfEleMissingHitsCut);

    return passID;

}


Selector::~Selector(){
}
