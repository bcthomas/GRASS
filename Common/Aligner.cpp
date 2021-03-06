/*
 * Common : a collection of classes (re)used throughout the scaffolder implementation.
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


#include "Aligner.h"
#include "Helpers.h"
#include "Globals.h"

// Base class destructor (removes alignment file).
Aligner::~Aligner()
{
    if (RemoveOutput && OutputFileName.length() > 0)
        Helpers::RemoveFile(OutputFileName);
}

// BWA Aligner ctor
BWAAligner::BWAAligner(const string &referenceFile, const string &queryFile, const BWAConfiguration &config)
	: Aligner(referenceFile, queryFile), Configuration(config)
{
	IndexFileExtensions.push_back(".amb");
	IndexFileExtensions.push_back(".ann");
	IndexFileExtensions.push_back(".bwt");
	IndexFileExtensions.push_back(".pac");
	IndexFileExtensions.push_back(".rbwt");
	IndexFileExtensions.push_back(".rpac");
	IndexFileExtensions.push_back(".rsa");
	IndexFileExtensions.push_back(".sa");
}

// Alignes query to reference using single end BWA alignement. Returns true if successful.
bool BWAAligner::Align(const string &outFile)
{
	char str[MaxLine];
	bool success = true;
	OutputFileName.clear();

	string prefix, outSai;
	prefix = Helpers::TempFile(Configuration.TmpPath);
	sprintf(str, Configuration.IndexCommand.c_str(), prefix.c_str(), ReferenceFileName.c_str());
	if (!Helpers::Execute(str))
		success = false;
	if (success)
	{
		outSai = Helpers::TempFile(Configuration.TmpPath);
		if (Configuration.ExactMatch)
			sprintf(str, Configuration.SuffixArrayExactCommand.c_str(), Configuration.NumberOfThreads, outSai.c_str(), prefix.c_str(), QueryFileName.c_str());
		else
			sprintf(str, Configuration.SuffixArrayCommand.c_str(), Configuration.NumberOfThreads, outSai.c_str(), prefix.c_str(), QueryFileName.c_str());
		if (!Helpers::Execute(str))
			success = false;
	}
	if (success)
	{
		OutputFileName = (outFile.length() > 0 ? outFile : Helpers::TempFile(Configuration.TmpPath));
		sprintf(str, Configuration.AlignSingleEndCommand.c_str(), OutputFileName.c_str(), Configuration.MaximumHits + 1, prefix.c_str(), outSai.c_str(), QueryFileName.c_str());
		if (!Helpers::Execute(str))
			success = false;
	}

	removeIndexFiles(prefix);
	if (outSai.length() > 0)
		Helpers::RemoveFile(outSai);
	if (!success && OutputFileName.length() > 0)
	{
		Helpers::RemoveFile(OutputFileName);
		OutputFileName.clear();
	}

	return success;
}

// Removes index files of BWA aligner with given prefix
void BWAAligner::removeIndexFiles(const string &prefix)
{
	int nSuffix = IndexFileExtensions.size();
	for (int i = 0; i < nSuffix; i++)
		Helpers::RemoveFile(prefix + IndexFileExtensions[i]);
}

vector<string> BWAAligner::IndexFileExtensions;

// Alignes query to reference using single end NovoAlign alignement. Returns true if successful.
bool NovoAlignAligner::Align(const string &outFile)
{
	char str[MaxLine];
	bool success = true;
	OutputFileName.clear();

	string prefix;
	prefix = Helpers::TempFile(Configuration.TmpPath);
	sprintf(str, Configuration.IndexCommand.c_str(), prefix.c_str(), ReferenceFileName.c_str());
	if (!Helpers::Execute(str))
		success = false;
	if (success)
	{
		OutputFileName = (outFile.length() > 0 ? outFile : Helpers::TempFile(Configuration.TmpPath));
		sprintf(str, Configuration.AlignSingleEndCommand.c_str(), prefix.c_str(), QueryFileName.c_str(), OutputFileName.c_str());
		if (!Helpers::Execute(str))
                    success = false;
	}

	Helpers::RemoveFile(prefix);
	if (!success)
	{
		Helpers::RemoveFile(OutputFileName);
		OutputFileName.clear();
	}

	return success;
}

// Aligns query to reference using MUMMER package. Returns true if successful.
bool MummerAligner::Align(const string &outFile)
{
    char str[MaxLine];
    bool success = true;
    OutputFileName.clear();

    string prefix = Helpers::TempFile(Configuration.TmpPath);
    string delta = Helpers::TempFile(Configuration.TmpPath);
    
    sprintf(str, Configuration.NucmerCommand.c_str(), prefix.c_str(), ReferenceFileName.c_str(), QueryFileName.c_str());
    if (!Helpers::Execute(str))
        success = false;
    if (success)
    {
        sprintf(str, Configuration.DeltaFilterCommand.c_str(), (prefix + ".delta").c_str(), delta.c_str());
        if (!Helpers::Execute(str))
            success = false;
    }
    if (success)
    {
        OutputFileName = (outFile.length() > 0 ? outFile : Helpers::TempFile(Configuration.TmpPath));
        sprintf(str, Configuration.ShowCoordsCommand.c_str(), delta.c_str(), OutputFileName.c_str());
        //sprintf(str, Configuration.ShowCoordsCommand.c_str(), (prefix + ".delta").c_str(), OutputFileName.c_str());
        if (!Helpers::Execute(str))
            success = false;
    }

    Helpers::RemoveFile(prefix + ".delta");
    Helpers::RemoveFile(delta);
    //cout << "Delta: " << prefix + ".delta" << endl;
    //cout << "Delta-filtered: " << delta << endl;
    if (!success)
    {
        Helpers::RemoveFile(OutputFileName);
        OutputFileName.clear();
    }

    return success;
}

// Aligns query to reference using MUMMER package and creates a tiling. Returns true if successful.
bool MummerTiler::Align(const string &outFile)
{
    char str[MaxLine];
    bool success = true;
    OutputFileName.clear();

    string prefix = Helpers::TempFile(Configuration.TmpPath);
    
    sprintf(str, Configuration.NucmerCommand.c_str(), prefix.c_str(), ReferenceFileName.c_str(), QueryFileName.c_str());
    if (!Helpers::Execute(str))
        success = false;
    if (success)
    {
        OutputFileName = (outFile.length() > 0 ? outFile : Helpers::TempFile(Configuration.TmpPath));
        sprintf(str, Configuration.ShowTilingCommand.c_str(), (prefix + ".delta").c_str(), OutputFileName.c_str());
        if (!Helpers::Execute(str))
            success = false;
    }

    Helpers::RemoveFile(prefix + ".delta");
    if (!success)
    {
        Helpers::RemoveFile(OutputFileName);
        OutputFileName.clear();
    }

    return success;
}
