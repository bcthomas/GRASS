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

#include "Globals.h"
#include "Reader.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

using namespace std;

Reader::Reader()
{
    fin = NULL;
    line = new char[MaxLine];
    buf = new char[MaxLine];
    num_reads = -1;
}

bool Reader::Open(const string &filename, const string &mode)
{
    if (fin != NULL)
        return false;
    fin = fopen(filename.c_str(), mode.c_str());
    if (fin == NULL)
        return false;
    return true;
}

bool Reader::Close()
{
    if (fin != NULL)
    {
        fclose(fin);
        fin = NULL;
        num_reads = -1;
        return true;
    }
    return false;
}

bool Reader::IsOpen() const
{
	return fin != NULL;
}

Reader::~Reader()
{
    Close();
    delete [] line;
    delete [] buf;
}

bool FastAReader::Read(string &seq, string &comment)
{
    if (fgets(line, MaxLine, fin) == NULL)
        return false;

    if (line[0] != '>')
        throw runtime_error("FastAReader : comment is not in correct format.");

    line[strlen(line) - 1] = '\0';
    comment = line + 1;
	seq.clear();

    int offset = 0;
    while (fgets(line, MaxLine, fin) != NULL)
    {
        if (line[0] == '>')
        {
            fseek(fin, -strlen(line), SEEK_CUR);
            break;
        }

        int len = strlen(line);
        while (len > 0 && isspace(line[len - 1]))
            --len;
        line[len] = '\0';

        if (len + offset + 1 > MaxLine)
        {
            seq += buf;
            offset = 0;
        }

        strcpy(buf + offset, line);
        offset += len;
    }

    if (offset > 0)
    {
        seq += buf;
        offset = 0;
    }

    for (int i = 0; i < (int)seq.length(); ++i)
        seq[i] = toupper(seq[i]);

    return true;
}

bool FastAReader::Read(FastASequence &seq)
{
    return Read(seq.Nucleotides, seq.Comment);
}

long long FastAReader::Read(vector<FastASequence> &sequences)
{
    sequences.resize(NumReads());
    string seq;
    string comment;
    for (unsigned i = 0; i < sequences.size(); ++i)
    {
        Read(seq, comment);
	sequences[i] = FastASequence(seq, comment);
    }

    return sequences.size();
}

long long FastAReader::NumReads()
{
    if (!IsOpen())
        return 0;
    long long index = ftell(fin);

    if (num_reads >= 0)
        return num_reads;

    fseek(fin, 0, SEEK_SET);
    long long num = 0;
    while (fgets(line, MaxLine, fin) != NULL)
    {
        if (line[0] == '>')
            ++num;
    }

    fseek(fin, index, SEEK_SET);
    return num_reads = num;
}

bool FastQReader::Read(string &seq, string &comment)
{
	string quality;
	return Read(seq, comment, quality);
}

bool FastQReader::Read(string &seq, string &comment, string &quality)
{
    if (fgets(line, MaxLine, fin) == NULL)
        return false;

    if (line[0] != '@')
		throw runtime_error("FastQReader : comment is not in correct format.");

    line[strlen(line) - 1] = '\0';
    comment = line + 1;
    seq.clear();
	quality.clear();

    int offset = 0;
    while (fgets(line, MaxLine, fin) != NULL)
    {
        if (line[0] == '+')
        {
			// read the comment
            fgets(line, MaxLine, fin);
            line[strlen(line) - 1] = '\0';
			// read single line (must be shorter than MaxLine) of quality scores
			int len = strlen(line);
			while (len > 0 && isspace(line[len - 1]))
				--len;
			line[len] = '\0';
			quality += line;

            break;
        }

        int len = strlen(line);
        while (len > 0 && isspace(line[len - 1]))
            --len;
        line[len] = '\0';

        if (len + offset + 1 > MaxLine)
        {
            seq += buf;
            offset = 0;
        }

        strcpy(buf + offset, line);
        offset += len;
    }

    if (offset > 0)
    {
        seq += buf;
        offset = 0;
    }

    for (int i = 0; i < (int)seq.length(); ++i)
        seq[i] = toupper(seq[i]);

    return true;
}

bool FastQReader::Read(FastASequence &seq)
{
    return Read(seq.Nucleotides, seq.Comment);
}

bool FastQReader::Read(FastQSequence &seq)
{
    return Read(seq.Nucleotides, seq.Comment, seq.Quality);
}

long long FastQReader::Read(vector<FastASequence> &sequences)
{
    sequences.resize(NumReads());
    string seq;
    string comment;
    for (unsigned i = 0; i < sequences.size(); ++i)
    {
        Read(seq, comment);
		sequences[i] = FastASequence(seq, comment);
    }

    return sequences.size();
}

long long FastQReader::Read(vector<FastQSequence> &sequences)
{
    sequences.resize(NumReads());
    string seq;
    string comment;
	string quality;
    for (unsigned i = 0; i < sequences.size(); ++i)
    {
        Read(seq, comment, quality);
		sequences[i] = FastQSequence(seq, comment, quality);
    }

    return sequences.size();
}

long long FastQReader::NumReads()
{
    if (!IsOpen())
        return 0;
    long long index = ftell(fin);

    if (num_reads >= 0)
        return num_reads;

    fseek(fin, 0, SEEK_SET);
    long long num = 0;
    while (fgets(line, MaxLine, fin) != NULL)
    {
        if (line[0] == '@')
            ++num;
    }

    fseek(fin, index, SEEK_SET);
    return num_reads = num;
}
