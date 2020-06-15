#ifndef ATOP_H_IN
#define ATOP_H_IN

#include <algorithm>
#include <array>
#include <future>
#include <map>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "util.h"

extern bool VERBOSE;

namespace atop
{
// Type denoting the output of a shell command
using shell_out_t = std::vector<std::string>;

enum class Frameworks : int
{
	tflite = 0,
	mlperf,
	SNPE
};

std::map<std::string, Frameworks> const frameworks_table
    = {{"tflite", Frameworks::tflite},
       {"mlperf", Frameworks::mlperf},
       {"SNPE", Frameworks::SNPE}};

inline Frameworks string2framework( std::string fr )
{
	auto it = frameworks_table.find( fr );
	if( it != frameworks_table.end() )
	{
		return it->second;
	}
	else
		throw std::runtime_error( fmt::format( "Enum {0} not defined", fr ) );
}

inline std::string framework2string( Frameworks fr )
{
	auto it = std::find_if( std::begin( frameworks_table ),
	                        std::end( frameworks_table ),
	                        [&]( auto&& p ) { return p.second == fr; } );

	if( it != std::end( frameworks_table ) )
	{
		return it->first;
	}
	else
		throw std::runtime_error( "Framework not defined" );
}

void check_reqs();
shell_out_t check_console_output( std::string const& cmd );
shell_out_t get_models_on_device( Frameworks );

// TODO: use std::variant for options?
std::future<shell_out_t>
run_tflite_benchmark( std::vector<std::string> const& model_paths,
                      std::map<std::string, std::string> const& options,
                      int processes = 1 );
std::future<shell_out_t>
run_snpe_benchmark( std::vector<std::string> const& model_paths,
                    std::map<std::string, std::string> const& options,
                    int processes = 1, int num_runs = 1 );

enum class DmesgProbes : int
{
	IOCTL = 0,
	TIME  = 1,

	// End marker; do not add anything below
	LAST = 2
};

std::map<std::string, DmesgProbes> const dmesg_probes_table
    = {{"IOCTL", DmesgProbes::IOCTL}, {"TIME", DmesgProbes::TIME}};

#define PROBE_IDX( probe_ ) static_cast<size_t>( ( probe_ ) )

inline std::string dmesgProbes2string( DmesgProbes pr )
{
	auto it = std::find_if( std::begin( dmesg_probes_table ),
	                        std::end( dmesg_probes_table ),
	                        [&]( auto&& p ) { return p.second == pr; } );

	if( it != std::end( dmesg_probes_table ) )
	{
		return it->first;
	}
	else
		throw std::runtime_error( "Dmesg probe not defined" );
}

inline DmesgProbes string2dmesgProbes( std::string pr )
{
	auto it = dmesg_probes_table.find( pr );
	if( it != dmesg_probes_table.end() )
	{
		return it->second;
	}
	else
		throw std::runtime_error( fmt::format( "Enum {0} not defined", pr ) );
}

using ioctl_dmesg_t
    = std::array<std::vector<std::string>, PROBE_IDX( DmesgProbes::LAST )>;

class IoctlDmesgStreamer
{
   public:
	explicit IoctlDmesgStreamer( std::vector<std::string> const& probes
	                             = {dmesgProbes2string( DmesgProbes::IOCTL ),
	                                dmesgProbes2string( DmesgProbes::TIME )} );
	~IoctlDmesgStreamer()                           = default;
	IoctlDmesgStreamer( IoctlDmesgStreamer const& ) = delete;
	IoctlDmesgStreamer( IoctlDmesgStreamer&& )      = delete;

	ioctl_dmesg_t const& get_data() { return this->latest_data; }
	std::map<std::string, int> const& get_interactions()
	{
		return this->latest_interactions;
	}

	ioctl_dmesg_t const& more();
	std::map<std::string, int> const& interactions( bool check_full_log = false,
	                                                double threshold = 20.0 );

	DmesgProbes utilization_probe;

   private:
	double latest_ts;
	ioctl_dmesg_t latest_data;
	std::map<std::string, int> latest_interactions;
	std::vector<std::string> probes;
};

class CpuUtilizationStreamer
{
   public:
	CpuUtilizationStreamer();
	~CpuUtilizationStreamer()                               = default;
	CpuUtilizationStreamer( CpuUtilizationStreamer const& ) = delete;
	CpuUtilizationStreamer( CpuUtilizationStreamer&& )      = delete;

	std::map<std::string, double> const& utilizations();

   private:
	std::vector<uint64_t> total_tick;
	std::vector<uint64_t> total_tick_old;
	std::vector<uint64_t> idle;
	std::vector<uint64_t> idle_old;
	std::vector<uint64_t> del_total_tick;
	std::vector<uint64_t> del_idle;

	std::map<std::string, double> latest_utils;

	int num_cpus;
};

struct BenchmarkStats
{
	// Latencies in microseconds
	std::unordered_map<std::string, uint64_t> stats
	    = {{"preproc", 0},
	       {"postproc", 0},
	       {"inference", 0},
	       {"offload", 0},
	       // Framework overhead for delegation
	       {"delegation", 0},
	       {"init", 0}};
};

void summarize_benchmark_output( shell_out_t const&, atop::Frameworks,
                                 BenchmarkStats& );

} // namespace atop

#endif // ATOP_H_IN
