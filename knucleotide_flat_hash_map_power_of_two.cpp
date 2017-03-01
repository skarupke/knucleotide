/* The Computer Language Benchmarks Game
   http://benchmarksgame.alioth.debian.org/

   Contributed by Branimir Maksimovic
*/

// g++ 4.8.x bug, compile with: -Wl,--no-as-needed option 

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

constexpr const unsigned char tochar[4] = { 'A', 'C', 'T', 'G' };

uint64_t mask_for_size(unsigned size)
{
   return (1llu << (2llu * size)) - 1llu;
}

struct T{
   T()
      : data(0)
   {
   }
   T(const std::string& s)
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
      data |= (c >> 1) & 0b11;
   }
   bool operator<(const T& in)const
   {
      return data < in.data;
   }
   bool operator==(const T& in)const
   {
      return data == in.data;
   }
   std::string to_string(unsigned size)const
   {
      std::string tmp;
      uint64_t tmp1 = data;
      for(unsigned i = 0; i != size; ++i)
      {
         tmp += tochar[tmp1 & 3];
         tmp1 >>= 2;
      }
      std::reverse(tmp.begin(),tmp.end());
      return tmp;
   }
   struct hash
   {
      uint64_t operator()(const T& t)const
      {
         return t.data;
      }
	  typedef ska::power_of_two_hash_policy hash_policy;
   };
   uint64_t data;
};

ska::flat_hash_map<T,unsigned,T::hash> calculate(const char * begin, const char * end, unsigned size)
{
   ska::flat_hash_map<T,unsigned,T::hash> frequencies;
   T tmp;
   uint64_t mask = mask_for_size(size);
   for (const char * init_end = begin + size - 1; begin != init_end; ++begin)
   {
      tmp.push(*begin, mask);
   }
   for (; begin != end; ++begin)
   {
      tmp.push(*begin, mask);
      ++frequencies[tmp];
   }
   return frequencies;
}

ska::flat_hash_map<T,unsigned,T::hash> tcalculate(const std::string& input,unsigned size)
{
   unsigned N = sysconf (_SC_NPROCESSORS_ONLN) - 1;

   std::vector<std::future<ska::flat_hash_map<T,unsigned,T::hash>>> ft(N);
   const char * begin = input.c_str();
   const char * end = input.c_str() + input.size();
   unsigned per_thread = input.size() / N;
   for(unsigned i = 1; i < N; ++i)
   {
      const char * this_end = begin + per_thread + size - 1;
      ft[i] = std::async(std::launch::async, &calculate, begin, this_end, size);
      begin += per_thread;
   }

   auto frequencies = calculate(begin, end, size);

   for(unsigned i = 1; i < N; ++i)
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
   std::future<unsigned> GGTATTTTAATTTATAGT = std::async(std::launch::async, compute_count, input, "GGTATTTTAATTTATAGT");
   std::future<unsigned> GGTATTTTAATT = std::async(std::launch::async, compute_count, input, "GGTATTTTAATT");
   std::future<unsigned> GGTATT = std::async(std::launch::async, compute_count, input, "GGTATT");
   std::future<unsigned> GGTA = std::async(std::launch::async, compute_count, input, "GGTA");
   std::future<unsigned> GGT = std::async(std::launch::async, compute_count, input, "GGT");
   write_frequencies(input,1);
   write_frequencies(input,2);
   write_single_count(GGT.get(), "GGT");
   write_single_count(GGTA.get(), "GGTA");
   write_single_count(GGTATT.get(), "GGTATT");
   write_single_count(GGTATTTTAATT.get(), "GGTATTTTAATT");
   write_single_count(GGTATTTTAATTTATAGT.get(), "GGTATTTTAATTTATAGT");
}

