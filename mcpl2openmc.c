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




void write_source_point(const char* filename, bool surf_source_bank)
{
  // When using parallel HDF5, the file is written to collectively by all
  // processes. With MPI-only, the file is opened and written by the master
  // (note that the call to write_source_bank is by all processes since slave
  // processes need to send source bank data to the master.

  std::string filename_;
  if (filename) {
    filename_ = filename;
  } else {
    // Determine width for zero padding
    int w = std::to_string(settings::n_max_batches).size();

    filename_ = fmt::format("{0}source.{1:0{2}}.h5", settings::path_output,
      simulation::current_batch, w);
  }

  hid_t file_id;
  file_id = file_open(filename_, 'w', true);
  write_attribute(file_id, "filetype", "source");

  // Get pointer to source bank and write to file
  write_source_bank(file_id, surf_source_bank);

  file_close(file_id);
}

/*This bit is copied over from openmc*/


void write_source_bank(hid_t group_id, bool surf_source_bank)
{
  hid_t banktype = h5banktype();

  // Set total and individual process dataspace sizes for source bank
  int64_t dims_size = settings::n_particles;
  int64_t count_size = simulation::work_per_rank;

  // Set vectors for source bank and starting bank index of each process
  vector<int64_t>* bank_index = &simulation::work_index;
  vector<SourceSite>* source_bank = &simulation::source_bank;
  vector<int64_t> surf_source_index_vector;
  vector<SourceSite> surf_source_bank_vector;

  // Reset dataspace sizes and vectors for surface source bank
  if (surf_source_bank) {
    surf_source_index_vector = calculate_surf_source_size();
    dims_size = surf_source_index_vector[mpi::n_procs];
    count_size = simulation::surf_source_bank.size();

    bank_index = &surf_source_index_vector;

    // Copy data in a SharedArray into a vector.
    surf_source_bank_vector.resize(count_size);
    surf_source_bank_vector.assign(simulation::surf_source_bank.data(),
      simulation::surf_source_bank.data() + count_size);
    source_bank = &surf_source_bank_vector;
  }

  if (mpi::master) {
    // Create dataset big enough to hold all source sites
    hsize_t dims[] {static_cast<hsize_t>(dims_size)};
    hid_t dspace = H5Screate_simple(1, dims, nullptr);
    hid_t dset = H5Dcreate(group_id, "source_bank", banktype, dspace,
      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Save source bank sites since the array is overwritten below

    for (int i = 0; i < mpi::n_procs; ++i) {
      // Create memory space
      hsize_t count[] {
        static_cast<hsize_t>((*bank_index)[i + 1] - (*bank_index)[i])};
      hid_t memspace = H5Screate_simple(1, count, nullptr);

      // Select hyperslab for this dataspace
      dspace = H5Dget_space(dset);
      hsize_t start[] {static_cast<hsize_t>((*bank_index)[i])};
      H5Sselect_hyperslab(
        dspace, H5S_SELECT_SET, start, nullptr, count, nullptr);

      // Write data to hyperslab
      H5Dwrite(
        dset, banktype, memspace, dspace, H5P_DEFAULT, (*source_bank).data());

      H5Sclose(memspace);
      H5Sclose(dspace);
    }

    // Close all ids
    H5Dclose(dset);

  } else {
  }

  H5Tclose(banktype);
}
