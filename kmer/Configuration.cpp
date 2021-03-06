/*
 * kmer : creates k-mer count statistics for a given k.
 * Copyright (C) 2011  Alexey Gritsenko
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 * 
 * 
 * 
 * Email: a.gritsenko@tudelft.nl
 * Mail: Delft University of Technology
 *       Faculty of Electrical Engineering, Mathematics, and Computer Science
 *       Department of Mediamatics
 *       P.O. Box 5031
 *       2600 GA, Delft, The Netherlands
 */

#include "Configuration.h"
#include "Defines.h"
#include "Helpers.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>

using namespace std;

// Construtor with default configuration parameter settings.
Configuration::Configuration()
{
	Success = false;
	Kmer = 36;
	Verbose = false;
	InputFileName = "";
}

// Parses command line arguments. Returns true if successful.
bool Configuration::ProcessCommandLine(int argc, char *argv[])
{
	this->Success = true;
	stringstream serr;

	if (argc == 1)
	{
		serr << "[-] Not enough arguments. Consult -help." << endl;
		this->Success = false;
	}
	else
	{
		int i = 1;
		while (i < argc)
		{
			if (!strcmp("-help", argv[i]) || !strcmp("-h", argv[i]))
			{
				printHelpMessage(serr);
				this->Success = false;
				break;
			}
			else if (!strcmp("-kmer", argv[i]))
			{
				if (argc - i - 1 < 1)
				{
					cerr << "[-] Parsing error in -kmer: must have an argument." << endl;
					this->Success = false;
					break;
				}
				i++;
				bool kmerSuccess;
				int kmer = Helpers::ParseInt(argv[i], kmerSuccess);
				if (!kmerSuccess || kmer <= 0)
				{
					cerr << "[-] Parsing error in -kmer: length must be a positive number." << endl;
					this->Success = false;
					break;
				}
				Kmer = kmer;
			}
			else if (!strcmp("-verbose", argv[i]))
			{
				if (argc - i - 1 < 1)
				{
					cerr << "[-] Parsing error in -verbose: must have an argument." << endl;
					this->Success = false;
					break;
				}
				i++;
				bool sw = false;
				if (!strcasecmp(argv[i], "yes"))
					sw = true;
				else if (!strcasecmp(argv[i], "no"))
					sw = false;
				else
				{
					cerr << "[-] Parsing error in -verbose: argument must be yes/no." << endl;
					this->Success = false;
					break;
				}
				Verbose = sw;
			}
			else if (i == argc - 1)
				this->InputFileName = argv[argc - 1];
			else
			{
				serr << "[-] Unknown argument: " << argv[i] << endl;
				this->Success = false;
				break;
			}
			i++;
		}
		if (this->InputFileName == "")
		{
			serr << "[-] No input file specified." << endl;
			this->Success = false;
		}
	}
	if (!this->Success)
		LastError = serr.str();
	return this->Success;
}

void Configuration::printHelpMessage(stringstream &serr)
{
	serr << "[i] Scaffold optimizer version " << VERSION << " (" << DATE << ")" << endl;
	serr << "[i] By " << AUTHOR << endl;
	serr << "[i] Usage: scaffoldOptimizer [arguments] <scaffold.opt>" << endl;
	serr << "[i] -help                                               Print this message and exit." << endl;
	serr << "[i] -kmer <length>                                      K-mer length for the statistics. [36]" << endl;
	serr << "[i] -verbose <yes/no>                                   Verbose statistics output. [no]" << endl;
}
