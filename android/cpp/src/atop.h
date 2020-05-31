#ifndef ATOP_H_IN
#define ATOP_H_IN

#include <string>
#include <vector>
#include <unordered_map>

#include "util.h"

extern bool VERBOSE;

namespace atop
{
void check_reqs();
std::vector<std::string> check_console_output( std::string const& cmd );

class IoctlDmesgStreamer
{
   public:
	IoctlDmesgStreamer();
	~IoctlDmesgStreamer()                           = default;
	IoctlDmesgStreamer( IoctlDmesgStreamer const& ) = delete;
	IoctlDmesgStreamer( IoctlDmesgStreamer&& )      = delete;

	std::vector<std::string> const& get_data() { return this->latest_data; }
	std::unordered_map<std::string, int> const& get_interactions() { return this->latest_interactions; }

	std::vector<std::string> const& more();
	std::unordered_map<std::string, int> const& interactions(bool check_full_log = false, double threshold = 20.0);

   private:
	double latest_ts;
	std::vector<std::string> latest_data;
	std::unordered_map<std::string, int> latest_interactions;
};

} // namespace atop

#endif // ATOP_H_IN
