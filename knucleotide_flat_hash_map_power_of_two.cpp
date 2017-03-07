/* The Computer Language Benchmarks Game
   http://benchmarksgame.alioth.debian.org/

   Contributed by Branimir Maksimovic
*/

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdio>
#include <string>
#include <cstring>
#include <algorithm>
#include <map>
#include "flat_hash_map.hpp"
#include <future>
#include <unistd.h>
#include <ext/pb_ds/assoc_container.hpp>

constexpr const unsigned char tochar[8] = { 'A', 'A', 'C', 'C', 'T', 'T', 'G', 'G' };

uint64_t mask_for_size(unsigned size)
{
   return (1llu << ((size << 1llu) + 1llu)) - 1llu;
}

struct RunningHash
{
   RunningHash()
      : data(0)
   {
   }
   RunningHash(const std::string& s)
      : data(0)
   {
      uint64_t mask = mask_for_size(s.size());
      for (char c : s)
      {
         push(c, mask);
      }
   }
   void push(char c, uint64_t mask)
   {
      data <<= 2;
      data &= mask;
      data |= c & 0b110;
   }
   bool operator<(const RunningHash & in)const
   {
      return data < in.data;
   }
   bool operator==(const RunningHash & in)const
   {
      return data == in.data;
   }
   std::string to_string(unsigned size)const
   {
      std::string tmp;
      uint64_t tmp1 = data;
      for(unsigned i = 0; i != size; ++i)
      {
         tmp += tochar[tmp1 & 0b110];
         tmp1 >>= 2;
      }
      std::reverse(tmp.begin(), tmp.end());
      return tmp;
   }
   struct hash
   {
      uint64_t operator()(const RunningHash & t) const
      {
         return t.data;
      }
	  typedef ska::power_of_two_hash_policy hash_policy;
   };
   uint64_t data;
};

#define MULTI_THREADED
#ifdef MULTI_THREADED
unsigned NUM_THREADS = sysconf (_SC_NPROCESSORS_ONLN) - 1;
std::launch launch_policy = std::launch::async;
#else
unsigned NUM_THREADS = 0;
std::launch launch_policy = std::launch::deferred;
#endif

//typedef __gnu_pbds::cc_hash_table<RunningHash, unsigned, RunningHash::hash> HashMap;
typedef ska::flat_hash_map<RunningHash, unsigned, RunningHash::hash> HashMap;

HashMap calculate(const char * begin, const char * end, unsigned size)
{
   HashMap frequencies;
   RunningHash tmp;
   uint64_t mask = mask_for_size(size);
   for (const char * init_end = begin + size - 1; begin != init_end; ++begin)
   {
      tmp.push(*begin, mask);
   }
   for (; begin != end; ++begin)
   {
      tmp.push(*begin, mask);
      ++frequencies[RunningHash(tmp)];
   }
   return frequencies;
}

HashMap tcalculate(const std::string& input,unsigned size)
{
   unsigned NUM_THREADS = sysconf (_SC_NPROCESSORS_ONLN) - 1;

   std::vector<std::future<HashMap>> ft(NUM_THREADS);
   const char * begin = input.c_str();
   const char * end = input.c_str() + input.size();
   unsigned per_thread = input.size() / NUM_THREADS;
   for(unsigned i = 1; i < NUM_THREADS; ++i)
   {
      const char * this_end = begin + per_thread + size - 1;
      ft[i] = std::async(launch_policy, &calculate, begin, this_end, size);
      begin += per_thread;
   }

   auto frequencies = calculate(begin, end, size);

   for(unsigned i = 1; i < NUM_THREADS; ++i)
   {
      for(auto && j : ft[i].get())
      {
         frequencies[j.first] += j.second;
      }
   }
   return frequencies;
}

void write_frequencies(const std::string & input, unsigned size)
{
   unsigned sum = input.size() + 1 - size;
   auto frequencies = tcalculate(input,size);
   std::vector<std::pair<unsigned, std::string>> freq_sorted;
   freq_sorted.reserve(frequencies.size());
   for(auto && i: frequencies)
   {
      freq_sorted.emplace_back(i.second, i.first.to_string(size));
   }
   std::sort(freq_sorted.begin(), freq_sorted.end(), [](const auto & l, const auto & r){ return l.first > r.first; });
   for(auto && i : freq_sorted)
      std::cout << i.second << ' ' << (sum ? double(100 * i.first) / sum : 0.0) << '\n';
   std::cout << '\n';
}

unsigned compute_count(const std::string & input, const std::string& string)
{
   return calculate(input.c_str(), input.c_str() + input.size(), string.size())[string];
   //return tcalculate(input, string.size())[string];
}

void write_single_count(unsigned count, const std::string & string)
{
   std::cout << count << '\t' << string << '\n';
}
void write_single_count(unsigned count, const char * string)
{
   std::cout << count << '\t' << string << '\n';
}

void write_count(const std::string & input, const std::string& string)
{
   write_single_count(tcalculate(input, string.size())[string], string);
}

int main()
{
   std::string input;
   char buffer[256];
   while (fgets(buffer,100,stdin) && memcmp(">THREE",buffer,6)!=0);
   while (fgets(buffer,100,stdin) && buffer[0] != '>')
   {
      if (buffer[0] != ';')
      {
         input.append(buffer,strlen(buffer)-1);
      }
   }

   std::cout << std::setprecision(3) << std::setiosflags(std::ios::fixed);
   std::future<unsigned> GGTATTTTAATTTATAGT = std::async(launch_policy, compute_count, input, "GGTATTTTAATTTATAGT");
   std::future<unsigned> GGTATTTTAATT = std::async(launch_policy, compute_count, input, "GGTATTTTAATT");
   std::future<unsigned> GGTATT = std::async(launch_policy, compute_count, input, "GGTATT");
   std::future<unsigned> GGTA = std::async(launch_policy, compute_count, input, "GGTA");
   std::future<unsigned> GGT = std::async(launch_policy, compute_count, input, "GGT");
   write_frequencies(input,1);
   write_frequencies(input,2);
   write_single_count(GGT.get(), "GGT");
   write_single_count(GGTA.get(), "GGTA");
   write_single_count(GGTATT.get(), "GGTATT");
   write_single_count(GGTATTTTAATT.get(), "GGTATTTTAATT");
   write_single_count(GGTATTTTAATTTATAGT.get(), "GGTATTTTAATTTATAGT");
}

