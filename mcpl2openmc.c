#include <math.h>
#include <cstdio>
#include <openmc/hdf5_interface>
#include <mcpl.h>
extern "C" {

  enum pdgcodes {PDGNEUTRON=2112, PDG_PHOTON=22, PDG_ELECTRON=11, PDG_POSITRON=-11};

const int map_openmc2pdg[]{PDG_NEUTRON,PDG_PHOTON,PDG_ELECTRON, PDG_POSITRON};

int map_pdg2openmc(int pdg_code){
  switch(pdg_code){
    case PDG_NEUTRON:
	return 0;
    case PDG_PHOTON:
	return 1;
    case PDG_ELECTRON:
	return 2;
    case PDG_POSITRON:
	return 3;
    default:
	return 0;
  }
}

int main (int argc, char *argv[]){
  /* Extract filename from input arguments.*/
  /*batch size for dropping "banks" of particles to h5*/
  const int batch_size=20;
  vector <SourceSite> openmc_ptcl_bank{batch_size};

  const char mcpl_filename="myfile.mcpl";
  const char openmc_filename="source_bank.h5";


  /* open the input MCPL-file for reading*/
  mcpl_file_t f = mcpl_open_file(mcpl_filename);
  const mcpl_particle_t* p;

  /* open the openmc output file */
  hit_t openmc_source_file = file_open(openmc_filename);


  long long particle_count;
  i=0;
  while ( ( p = mcpl_read(f) ) ) {
    int pdgc=p->pdgcode;
    if (ppdgc!=PDG_NEUTRON &&  pdgc!=PDG_PHOTON && pdgc!=PDG_ELECTRON && pdgc!=PDG_POSITRON){
      /*this is a particle type unknown to openmc - skip*/
      nonopenmc_particle_count;
      continue;
    }
    SourceSite omcp;
    omcp->ParticleType=map_pdg2openmc(pdgc);
    omcp->r=Position(p->position);
    omcp->u=Direction(p->direction);
    omcp->E=p->ekin*1e6;/*MCPL uses MeV, openmc eV*/
    omcp->wgt=p->weight;
    omcp->time=p->time;
    openmc_ptcl_bank[i]=ocmp;
    particle_count++;
    if (i==batchsize){
      /*flush the bank to file using openmc calls*/
      i=0;
    }
  }
  if(i!=0){
    /*particle count not divisible by batch size so flush the remaining particles*/

  }

  if (verbose){
    printf("INFO: MCPL-file %s contained %lu particles.\n",mcpl_filename,mcpl_particle_count);
    printf("INFO: OpenMC-file %s has received %lu particles.\n",omc_filename,particle_count);
    printf("IMFO: %lu particles were discarded.\n",nonopenmc_particle_count);
  }
  mcpl_close_file(f);
}





