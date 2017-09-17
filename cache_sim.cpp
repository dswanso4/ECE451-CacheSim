/*
Assignment: ECE 451 Programming Assignment 3
Date: November 30, 2016
Group: David Swanson, Daniel Caballero, Micheal Wilder

Description: This code simulates either a direct-mapped or set associative cache. The user
specifies the block size (in bytes), the number of blocks in the cache, the associativity 
that is desired, and a file with addresses in hexidecimal form. The program simulates how
the cache behaves given those parameters and addresses.
*/
#include <iostream>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <math.h>
using namespace std;

/* Function that gives useful information on how to run the code. This function is called
if there is some error when running the code. */

void usage() {
  printf("Code Usage:\n\n");
  printf("./cache_sim 1 16 1 LRU test.txt\n\n");
  printf("  parameter1 = block size (number of bytes)\n");
  printf("  parameter2 = number of blocks in the cache\n");
  printf("  parameter3 = associativity\n");
  printf("    - Input 1 for direct\n");
  printf("    - Associativity must be a factor of number of blocks\n");
  printf("  parameter4 = replacement policy (must be 'LRU' or 'Random')\n");
  printf("  parameter5 = file name with addresses\n");
}

/* Start of main function. All useful activity happens within this function. */

int main(int argc, char *argv[]) {
  // Direct-mapped variables
  int index;						// Number of bits used for the index
  vector <int> cache;				// The cache is represented as a vector for ease of use.
  vector <int> memory;				// Used to store the address associated to a cache index

  // Set-Associative variables
  int assoc;						// The type of associativity (1,2,4...)
  int temp;							// Temporary variable used to simplify cache access
  int num_sets;						// Number of sets
  int size;							// Records how many values a set has
  int tmp;							// Temporary variable used for swapping
  int random;						// Randomly generated number for Random replacement policy
  string policy;					// Specified replacement policy
  vector < vector <int> > a_cache;	// 2D cache since each set can have multiple data
  vector <int>::iterator it;		// Iterator used to search the sets

  // Variables common to both
  int i, j;							// Induction variables
  int block_size, num_blocks;		// Variables that hold the amount and size of blocks
  int offset;						// Number of bits used for the offset
  int misses, hits;					// Running count variables for hits and misses
  string line;						// String used to read from address file
  vector <int> address_list;		// Vector that stores addresses in decimal form
  ifstream file;					// Used to access address file

  // Paramaters are read into variables, offset is calculated, and hit/miss are initialized
  block_size = atoi(argv[1]);
  num_blocks = atoi(argv[2]);
  assoc = atoi(argv[3]);
  policy = argv[4];
  offset = log2(block_size);
  misses = 0;
  hits = 0;

  // Report and error if the replacement policy is incorrect
  if (policy != "LRU" && policy != "Random") {
	cout << "Invlaid Replacement policy!" << endl;
	usage();
	exit(0);
  }

  // The addresses are read from the file and converted to decimal form
  file.open(argv[5]);
  if (file.is_open()) {
    while (getline(file,line)) {
      sscanf(line.c_str(), "0x%x", &i);
      address_list.push_back(i);
	}
  }
  else {
    printf("Error opening file!\n");
	usage();
	exit(0);
  }
  file.close();
  
  // This is the start of the direct-mapped cache setting (when associativity = 1).
  if (assoc == 1) {
	index = log2(num_blocks);		// Index is calculated (2^index = num_blocks).
	cache.resize(num_blocks, -1);	// Cache is initialized to hold all possible indexes
	memory.resize(num_blocks, -1);

	/* Iterate through the list of addresses. The index in the cache where an address needs 
	   to go is calculated by the address without the offset modulo the number of blocks. If 
	   this index has the same tag, then it is a hit and the address is stored. Otherwise, it 
	   is a miss and the address is stored. */

    for(i = 0; i < address_list.size(); i++) {
	  cout << address_list[i];
	  if (cache[(address_list[i]>>offset)%num_blocks] == (address_list[i] >> (offset+index))) {
		memory[(address_list[i] >> offset) % num_blocks] = address_list[i];
		hits++;
        cout << " (hit)" << endl;
	  }
	  else {
		misses++;
	    cout << " (miss)" << endl;
	    cache[(address_list[i] >> offset) % (1 << index)] = address_list[i] >> (offset+index);
		memory[(address_list[i] >> offset) % num_blocks] = address_list[i];
	  }
    }

	/* The contents of the cache and the data associated with the hits and misses are printed */
	cout << "\nFinal Cache Contents: <index , data>\n";
	for (i = 0; i < memory.size(); i++) {
      if (cache[i] != -1) 
		cout << "<" << i << " , " << memory[i] << ">" << endl;
	}
    printf("\nReads: %d\n", address_list.size());
    printf("Hits: %d\n", hits);
    printf("Misses: %d\n", misses);
    printf("Hit Rate: %.2f%\n", ((1.0*hits)/(1.0*address_list.size()))*100.0);
    printf("Miss Rate: %.2f%\n", ((1.0*misses)/(1.0*address_list.size()))*100.0);
  }

  /* This is the start of the Set_Associative caches (when associativity = 2, 4, 8...). The 
     associativity must be a factor of the number of blocks in order to work. */
  else if (assoc != 0 && num_blocks%assoc == 0) {
	// The number of sets is calculated and the first demension of the cache is resized accordingly.
	num_sets = num_blocks/assoc;		
	a_cache.resize(num_sets);

	/* The list of addresses is iterated through. The address is associated to a set in the cache
	   by eliminating the offset and modulo by the number of sets. If that set already has the 
	   address in it, then it is a hit and nothing changes. Otherwise, it is a miss and address
	   is stored in the cache. If the set is full, then one value is replaced based on the specified
	   replacement policy. */

	for (i = 0; i < address_list.size(); i++) {
	  cout << address_list[i];
      temp = (address_list[i] >> offset) % num_sets;
	  size = a_cache[temp].size();
	  it = find (a_cache[temp].begin(), a_cache[temp].end(), address_list[i]);
	  if (it == a_cache[temp].end()) {
        misses++;
		cout << " (miss)" << endl;
		if (size == assoc) {
		  if (policy == "LRU") {
		    a_cache[temp][0] = a_cache[temp][size-1];
		    a_cache[temp].pop_back();
		  }
		  else {
			random = rand() % assoc;
			tmp = a_cache[temp][size-1];
			a_cache[temp][size-1] = a_cache[temp][random];
			a_cache[temp][random] = tmp;
			a_cache[temp].pop_back();
		  }
		}
		a_cache[temp].push_back(address_list[i]);
	  }
	  else {
		hits++;
		cout << " (hit)" << endl;
	  }
	}

	// The contents of the cache and the data associated with the hits and misses are printed.
	cout << "\nFinal Cache Contents: <Set , Data>\n";
	for (i = 0; i < a_cache.size(); i++) {
      for (j = 0; j < a_cache[i].size(); j++) {
        cout << "<" << i << " , " << a_cache[i][j] << ">" << endl;
	  }
	}
	printf("\nReads: %d\n", address_list.size());
    printf("Hits: %d\n", hits);
    printf("Misses: %d\n", misses);
    printf("Hit Rate: %.2f%\n", ((1.0*hits)/(1.0*address_list.size()))*100.0);
    printf("Miss Rate: %.2f%\n", ((1.0*misses)/(1.0*address_list.size()))*100.0);

  }

  // An error is printed if the associativity given does not work.
  else {
    printf("Invalid associativity parameter!\n");
    usage();
	exit(0);
  }
  return 0;
}
