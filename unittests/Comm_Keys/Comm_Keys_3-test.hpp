// ===================
//  Author: Peize Lin
//  date: 2022.06.22
// ===================

#pragma once

//#include "Comm_Keys/Comm_Keys_31-gather.h"
//#include "Comm_Keys/Comm_Keys_32-gather.h"
#include "Comm_Keys/Comm_Keys_31-sr.h"
#include "Comm_Keys/Comm_Keys_32-sr.h"
#include "example/Communicate_Set.h"
#include "unittests/print_stl.h"

#include <mpi.h>
#include <vector>
#include <set>
#include <cassert>
#include <fstream>

namespace Comm_Keys_3_Test
{
	class Require_Judge
	{
	public:
		Require_Judge()=default;
		Require_Judge(const MPI_Comm &mpi_comm)
		{
			MPI_Comm_size( mpi_comm, &rank_size );
			MPI_Comm_rank( mpi_comm, &rank_mine );
		}
		bool judge(const int &key) const
		{
			return key%(rank_size+1)==rank_mine;
		}
		template <class Archive> void serialize( Archive & ar ){ ar(rank_size, rank_mine); }
	private:
		int rank_size=-1;
		int rank_mine=-1;
	};

	class Provider_Judge
	{
	public:
		Provider_Judge()=default;
		Provider_Judge(const MPI_Comm &mpi_comm)
		{
			MPI_Comm_rank( mpi_comm, &rank_mine );
			if(rank_mine==0)
				v = {2,3,5,7,9};
			else if(rank_mine==1)
				v = {3,1,4};
			else if(rank_mine==2)
				v = {2};
			else
				v = {};
			std::ofstream ofs("out."+std::to_string(rank_mine), std::ofstream::app);
			ofs<<"v\t"<<v<<std::endl;
		}
		bool judge(const int &key) const
		{
			return v.find(key)!=v.end();
		}
		template <class Archive> void serialize( Archive & ar ){ ar(rank_mine, v); }
	private:
		std::set<int> v;
		int rank_mine=-1;
	};

	void main1(int argc, char *argv[])
	{
		int mpi_thread_provide;
		MPI_Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &mpi_thread_provide );
		assert(mpi_thread_provide==MPI_THREAD_MULTIPLE);
		int rank_mine;	MPI_Comm_rank( MPI_COMM_WORLD, &rank_mine );

		std::set<int> v;
		if(rank_mine==0)
			v = {2,3,5,7,9};
		else if(rank_mine==1)
			v = {3,1,4};
		else if(rank_mine==2)
			v = {2};
		else
			v = {};

		std::ofstream ofs("out."+std::to_string(rank_mine));
		ofs<<"v\t"<<v<<std::endl;

		Require_Judge require_judge(MPI_COMM_WORLD);
		Comm_Keys_31_SenderTraversal<int, std::set<int>, Require_Judge> comm(MPI_COMM_WORLD);
		//Comm_Keys_32_SenderTraversal<int, std::set<int>, Require_Judge> comm(MPI_COMM_WORLD);
		comm.traverse_keys_provide = Communicate_Set::traverse_keys<int>;
		std::vector<std::vector<int>> keys_trans_list = comm.trans(v, require_judge);

		ofs<<"keys_trans_list\t"<<keys_trans_list<<std::endl;

		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Finalize();
	}

	static void main2(int argc, char *argv[])
	{
		int mpi_thread_provide;
		MPI_Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &mpi_thread_provide );
		assert(mpi_thread_provide==MPI_THREAD_MULTIPLE);
		int rank_mine;	MPI_Comm_rank( MPI_COMM_WORLD, &rank_mine );
		
		std::ofstream ofs("out."+std::to_string(rank_mine));

		Provider_Judge provider_judge(MPI_COMM_WORLD);
		Require_Judge require_judge(MPI_COMM_WORLD);
		Comm_Keys_31_SenderJudge<int, Provider_Judge, Require_Judge> comm(MPI_COMM_WORLD);
		//Comm_Keys_32_SenderJudge<int, Provider_Judge, Require_Judge> comm(MPI_COMM_WORLD);
		comm.traverse_keys_all = [](std::function<void(const int&)> &func){ for(int i=-3; i<10; ++i) func(i); };
		std::vector<std::vector<int>> keys_trans_list = comm.trans(provider_judge, require_judge);

		ofs<<"keys_trans_list\t"<<keys_trans_list<<std::endl;

		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Finalize();
	}

	/*
	keys_trans_list

		mpirun -n 1
		0:	[2]

		mpirun -n 2
		0:	[3,9], [7]
		1:	[3], [1,4]

		mpirun -n 3
		0:	[], [5,9], [2]
		1:	[4], [1], []
		2:	[], [], [2]

		mpirun -n 4
		0:	[5], [], [2,7], [3]
		1:	[], [1], [], [3]
		2:	[], [], [2], []
		3:	[], [], [], []
	*/
}