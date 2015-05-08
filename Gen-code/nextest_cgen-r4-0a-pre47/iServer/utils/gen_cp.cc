#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
/*
 * gen_cp.cc	Generate Calling Plan
 *
 * Usage: gen_cp [-12dsmxvX] [-l number_length] [-o outfile] file
 *
 * command line options:
 *		-o outfile name <default file.xml>
 * 		-1 only generate route(s) for translating area_code into new_area_code <default>
 * 		-2 generate route(s) for translating area_code into new_area_code
 *		   and route(s) for the new_area_code
 *		-s use source routing
 *		-d use destination routing <default>
 *		-m use source routing for the routes which translate area_code to new_area_code and
 *		   destination routing for the the new_area_code routes
 *		-x use default source/destination lengths
 *		-l number length (ignored if end_number specified) <default 15>
 *		-v display program version
 *		-X generate debug output
 *
 *
 * Format of input file is:
 *		city, area_code, start_number, end_number, new_area_code, calling_plan, priority {, calling_plan, priority }
 *
 * Note:
 *		  i) the start_number and end_number do not include the area code
 *		 ii) if no end_number is given, then the number range is assumed to the form nnnn*
 *           where * is determined from the number length
 *		iii) the new_area_code is optional
 *		 iv) the calling_plan/priority may be followed by additional calling_plans/priorities
 *		  v) a reject route can be created by prefixing a - to the area_code
 *
 *
 * Copyright 2001 and 2002	NexTone Communications, Inc.
 *
 */

static const char rcsid[] = "$Id: gen_cp.cc,v 1.7 2002/05/06 20:17:28 soakley Exp $";

#include <stdio.h>
#include <iostream.h>
#include <strstream.h>
#include <fstream.h>
#include <utility>
#include <functional>
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>
#include <set>


//
// Prototypes
//

class NumberRange;
struct NumberRange_less;
class Route;
struct Route_less;
class CallingPlan;
struct CallingPlan_less;
class CallingPlanBinding;
struct CallingPlanBinding_less;
class CreateRoutes;
unsigned long long power(unsigned int, unsigned int);
void error(string, string);
void error(string, const int);
void usage();
void help();
void tokenizeString(const string&, vector<string>&, char);
void generateRoutes(CallingPlan*);
void emitCallingPlan(CallingPlan*);
void emitRoute(Route*);
void emitCallingPlanBindings(CallingPlanBinding*);
void showProgress();


const int max_city_name_len = 10;
const int max_calling_plan_name_len = 25;
const int default_number_len = 15;

time_t create_time = time(NULL);


int line_no = 0;

int total_number_ranges;

bool use_default_len = false;

bool debug = false;


#define DEBUG(x) if(debug) { x }


//
// Pre calculate powers of 10
//

unsigned long long power10[] = {
	power(10, 0),
	power(10, 1),
	power(10, 2),
	power(10, 3),
	power(10, 4),
	power(10, 5),
	power(10, 6),
	power(10, 7),
	power(10, 8),
	power(10, 9),
	power(10, 10),
	power(10, 11),
	power(10, 12),
	power(10, 13),
	power(10, 14),
	power(10, 15)
};


enum Routing { src, dest };


class NumberRange
{
	Routing routing;
	string city;
	string priority;
	string area_code;
	string start_number;
	string end_number;
	string new_area_code;

public:
	NumberRange(Routing routing, string city, string priority, string area, string start, string end, string new_area)
		: routing(routing), city(city), priority(priority), area_code(area), start_number(start), end_number(end), new_area_code(new_area) {}

	friend struct NumberRange_less;
	friend class CallingPlan;
	friend class CreateRoutes;
};


struct NumberRange_less
{
	bool operator() (const NumberRange* r1, const NumberRange* r2) {
		return string(r1->area_code + r1->start_number) < string(r2->area_code + r2->start_number);
	}
};


class Route
{
	string name;
	string src;
	unsigned int src_len;
	string dest;
	unsigned int dest_len;
	string prefix;
	string flags;

public:
	Route(string n, string s, unsigned int slen, string d, unsigned int dlen, string f, string p)
		: name(n), src(s), src_len(slen), dest(d), dest_len(dlen), flags(f), prefix(p) { }

	friend struct Route_less;
	friend class CallingPlan;
	friend void emitRoute(Route*);
};


struct Route_less
{
	bool operator() (const Route* r1, const Route* r2) {
		return r1->name < r2->name;
	}
};


set<Route*, Route_less> routes;


class CallingPlan
{
	string name;
	set<NumberRange*, NumberRange_less> number_ranges;
	set<Route*, Route_less> routes;

public:
	CallingPlan(string name)
		: name(name) {}
	void addNumberRange(Routing routing, string city, string priority, string area_code, string start, string end, string new_area_code);

	friend struct CallingPlan_less;
	friend void generateRoutes(CallingPlan*);
	friend void emitCallingPlan(CallingPlan*);
	friend void calcNumberRanges(CallingPlan*);
};


struct CallingPlan_less
{
	bool operator() (const CallingPlan* cp1, const CallingPlan* cp2) {
		return(cp1->name < cp2->name);
	}
};


set<CallingPlan*, CallingPlan_less> calling_plans;


void CallingPlan::addNumberRange(Routing routing, string city, string priority, string area_code, string start, string end, string new_area_code)
{
	DEBUG(cerr << "number range: " << area_code << "," << start << "," << end << "," << new_area_code << endl;)

	if(start.length() != end.length()) error("number range length error: line", line_no);

	unsigned long long a = strtoull(start.c_str(), NULL, 10);
	unsigned long long b = strtoull(end.c_str(), NULL, 10);

	if(b < a) error("invalid number range: line:", line_no);

	set<NumberRange*, NumberRange_less>::iterator iter;

	for(iter = number_ranges.begin(); iter != number_ranges.end(); ++iter) {
		DEBUG(cerr << "compare number range: " << (*iter)->area_code << ","<< (*iter)->start_number << "," << (*iter)->end_number << "," << (*iter)->new_area_code << endl;)

		if((*iter)->city != city) continue;

		if((*iter)->priority != priority) continue;

		if((*iter)->area_code != area_code) continue;

		if((*iter)->routing != routing) continue;

		unsigned long long x = strtoull((*iter)->start_number.c_str(), NULL, 10);
		unsigned long long y = strtoull((*iter)->end_number.c_str(), NULL, 10);

		if((b + 1) < x || a > (y + 1)) continue;

		if((*iter)->new_area_code != new_area_code) error("new area code error: line:", line_no);

		if(a < x) (*iter)->start_number = start;

		if(b > y) (*iter)->end_number = end;

		break;
	}

	if(iter == number_ranges.end())
		number_ranges.insert(new NumberRange(routing, city, priority, area_code, start, end, new_area_code));
}


class CallingPlanBinding
{
	string calling_plan_name;
	string route_name;
	string priority;

public:
	CallingPlanBinding(string cp, string r, string p)
		: calling_plan_name(cp), route_name(r), priority(p) {}
	
	friend struct CallingPlanBinding_less;
	friend void emitCallingPlanBindings(CallingPlanBinding*);
};


struct CallingPlanBinding_less
{
	bool operator() (const CallingPlanBinding* cprb1, const CallingPlanBinding* cprb2) {
		return(cprb1->calling_plan_name < cprb2->calling_plan_name);
	}
};


multiset<CallingPlanBinding*, CallingPlanBinding_less> calling_plan_bindings;


class CreateRoutes
{
	string calling_plan_name;

public:
	CreateRoutes(string name)
		: calling_plan_name(string(name, 0, max_calling_plan_name_len)) {}
	void operator()(NumberRange* number_range);
};


void CreateRoutes::operator()(NumberRange* number_range)
{
	DEBUG(cerr << "create routes: " << number_range->start_number << "," << number_range->end_number << endl;)

	unsigned long long x = strtoull(number_range->start_number.c_str(), NULL, 10);
	unsigned long long y = strtoull(number_range->end_number.c_str(), NULL, 10);
	unsigned long long range = y - x + 1;
	unsigned int n = 1, pad_len;
	string src(""), dest(""), prefix, flags;

	while(range >= 0) {
		unsigned long long increment = power10[n - 1];

		if(x % power10[n] != 0 || power10[n] > range) {
			do {
				DEBUG(cerr << "new route (size = " << power10[n - 1] << ")" << endl;)

				char buf[32];
				unsigned int len = number_range->start_number.length();
				unsigned int total_len = len + number_range->area_code.length();

				sprintf(buf, "%1$.*2$lld", x, len);
				buf[len - n + 1] = '\0';

				dest = number_range->area_code + buf;
				pad_len = total_len - dest.length();

				string route_name(number_range->city, 0, max_city_name_len);
				route_name.append('-' + dest);

				for(;pad_len > 0; --pad_len) route_name += 'x';

				if(number_range->routing == ::src) {
					flags = "16";
					route_name += 's';
				} else { 
					flags = "32";
					route_name += 'd';
				}

				string prefix(number_range->new_area_code + buf);

				if(string(prefix, 0, 1) == "-") prefix = "";

				int dest_len = use_default_len ? 0 : total_len;

				routes.insert(new Route(route_name, src, 0, dest, dest_len, flags, prefix));

				calling_plan_bindings.insert(new CallingPlanBinding(calling_plan_name, route_name, number_range->priority));

				x += increment;
				range -= increment;

				showProgress();

			} while(range >= increment && x % power10[n] != 0);
		}

		if(range == 0) break;

		++n;

		while(power10[n - 1] > range) --n;
	}
}


unsigned long long power(unsigned int x, unsigned int n)
{
	if(n == 0)
		return 1;
	else
		return x * power(x, n - 1);
}


void error(string s, string s2="")
{
	cerr << s << ' ' << s2  << endl;
	exit(1);
}


void error(string s, const int i)
{
	cerr << s << ' ' << i  << endl;
	exit(1);
}


void usage()
{
	cout << "Usage: gen_cp [-12dsmxX] [-l number_len] [-o outfile] file" << endl;
}


void help()
{
	usage();
	cout << endl;
	cout << "command line options:" << endl;
	cout << "       -o outfile name <default file.xml>" << endl;
	cout << "       -1 only generate route(s) for translating area_code into new_area_code <default>" << endl;
	cout << "       -2 generate route(s) for translating area_code into new_area_code" << endl;
	cout << "          and route(s) for the new_area_code" << endl;
	cout << "       -s use source routing" << endl;
	cout << "       -d use destination routing <default>" << endl;
	cout << "       -m use source routing for the routes which translate area_code to new_area_code and" << endl;
	cout << "          destination routing for the new_area_code routes" << endl;
	cout << "       -x use default source/destination lengths" << endl;
	cout << "       -l number length (ignored if end_number specified) <default 15>" << endl;
	cout << "       -v display program version" << endl;
	cout << "       -X generate debug output" << endl;
	cout << endl;
	cout << endl;
	cout << "Format of input file is:" << endl;
	cout << "    city, area_code, start_number, end_number, new_area_code, calling_plan, priority {, calling_plan, priority }" << endl;
	cout << endl;
	cout << "Note:" << endl;
	cout << "      i) the start_number and end_number do not include the area code" << endl;
	cout << "     ii) if no end_number is given, then the number range is assumed to the form nnnn*" << endl;
	cout << "         where * is determined from the number length" << endl;
 	cout << "    iii) the new_area_code is optional" << endl;
 	cout << "     iv) the calling_plan/priority may be followed by additional calling_plans/priorities" << endl;
	cout << "      v) a reject route can be created by prefixing a - to the area_code" << endl;
	cout << endl;

	exit(1);
}


void tokenizeString(const string& str, vector<string>& tokens, char delim)
{
	tokens.clear();
	string::const_iterator iter = str.begin();

	while(iter != str.end()) {
		string tmp(iter, str.end());
		string::size_type ndx = tmp.find_first_of(delim);
		if(string::npos == ndx) {
			tokens.push_back(tmp);
			break;
		}
		tokens.push_back(string(iter, iter + ndx));
		iter += (ndx + 1);
	}
}


void generateRoutes(CallingPlan* cp)
{
	for_each(cp->number_ranges.begin(), cp->number_ranges.end(), CreateRoutes(cp->name));
}


void emitCallingPlan(CallingPlan* cp)
{
	cout << "<CP>" << endl;
	cout << "<CP_NAME> \"" << cp->name << "\" </CP_NAME>" << endl;
	cout << "<VPN_GROUP> \"\" </VPN_GROUP>" << endl;
	cout << "<MTIME> \"" << create_time << "\" </MTIME>" << endl;
	cout << "</CP>" << endl;
	cout << endl;

	showProgress();
}


void emitRoute(Route* r)
{
	cout << "<CR>" << endl;
	cout << "<CR_NAME> \"" << r->name << "\" </CR_NAME>" << endl;
	cout << "<CR_SRC> \"" << r->src << "\" </CR_SRC>" << endl;
	cout << "<CR_SRCLEN> \"" << r->src_len << "\" </CR_SRCLEN>" << endl;
	cout << "<CR_SRCPREFIX> \"\" </CR_SRCPREFIX>" << endl;
	cout << "<CR_DEST> \"" << r->dest << "\" </CR_DEST>" << endl;
	cout << "<CR_DESTLEN> \"" << r->dest_len << "\" </CR_DESTLEN>" << endl;
	cout << "<CR_PREFIX> \"" << r->prefix << "\" </CR_PREFIX>" << endl;
	cout << "<CR_FLAGS> \"" << r->flags << "\" </CR_FLAGS>" << endl;
	cout << "<MTIME> \"" << create_time << "\" </MTIME>" << endl;
	cout << "</CR>" << endl;
	cout << endl;

	showProgress();
}


void emitCallingPlanBindings(CallingPlanBinding* cpb)
{
	cout << "<CPB>" << endl;
	cout << "<CP_NAME> \"" << cpb->calling_plan_name << "\" </CP_NAME>" << endl;
	cout << "<CR_NAME> \"" << cpb->route_name << "\" </CR_NAME>" << endl;
	cout << "<CR_FLAGS> \"0\" </CR_FLAGS>" << endl;
	cout << "<CR_STIME> \"-1/-1/-1/-1/-1/-1/-1/-1\" </CR_STIME>" << endl;
	cout << "<CR_FTIME> \"-1/-1/-1/-1/-1/-1/-1/-1\" </CR_FTIME>" << endl;
	cout << "<PRIO> \"" << cpb->priority << "\" </PRIO>" << endl;
	cout << "<MTIME> \"" << create_time << "\" </MTIME>" << endl;
	cout << "</CPB>" << endl;
	cout << endl;

	showProgress();
}


void calcNumberRanges(CallingPlan* cp)
{
	total_number_ranges += cp->number_ranges.size();
}


void showProgress()
{
	char star[] = { '/', '-', '\\', '|' };
	static int n = 0;

	if(!debug) {
		cerr << '\b' << star[n++];

		if(n >= sizeof(star)) n = 0;
	}
}


int main(int argc, char* argv[])
{
	const int min_number_of_tokens = 7;
	int c;
	int errflg = 0;
	string ifile;
	string ofile;
	Routing r1 = ::dest;
	Routing r2 = ::dest;
	int number_len = default_number_len;
	bool two_routes = false;

	// Process input options
	while((c = getopt(argc, argv, "Xvh12dsmxl:o:")) != EOF) {
		switch(c) {
			case '1':
				two_routes = false;
				break;

			case '2':
				two_routes = true;
				break;

			case 'd':
				r1 = ::dest;
				r2 = ::dest;
				break;

			case 's':
				r1 = ::src;
				r2 = ::src;
				break;

			case 'm':
				r1 = ::src;
				r2 = ::dest;
				break;

			case 'l':
				number_len = atoi(optarg);
				if(number_len < 1 || number_len > 30) error("invalid number length:", number_len);
				break;

			case 'o':
				ofile = string(optarg);
				break;

			case 'x':
				use_default_len = true;
				break;

			case 'X':
				debug = true;
				break;

			case 'v':
				error(rcsid);
				break;

			case 'h':
				help();
				break;

			case '?':
				++errflg;
				break;
		}
	}

	if(optind < argc)
		ifile = string(argv[optind]);
	else
		++errflg;

	if(errflg) {
		usage();
		exit(1);
	}

	// If not output filename was specified create output filename
	if(ofile == "") {
		string tmp(ifile);
		string::size_type ndx = tmp.find_last_of('.');

		if(ndx >= tmp.length())
			tmp.append(".xml");
		else
			tmp.replace(ndx, tmp.length() - ndx, ".xml");

		ofile = tmp;
	}

	// Open input file
	ifstream fin(ifile.c_str());
	if(!fin) error("cannot open input file", ifile);

	// Check ouput does not over write input
	if(ifile == ofile) error("outfile replaces infile:", ofile);

	// Open output file
	ofstream fout(ofile.c_str());
	if(!fout) error("cannot open output file:", ofile);

	// Redirect std out to output file
	streambuf* cout_sbuf = cout.rdbuf();
	cout.rdbuf(fout.rdbuf());

	// Read number ranges from input file
	string input;

	cerr << "\bCreating number range(s)" << endl;

	while(++line_no, getline(fin, input))
	{
		DEBUG(cerr << "input: " << input << endl;)

		vector<string> input_tokens;

		tokenizeString(input, input_tokens, ',');

		vector<string>::iterator token = input_tokens.begin();

		if(string(*token, 0 , 1) == "#") continue;

		if(input_tokens.size() < min_number_of_tokens) error("incomplete input: line: ", line_no);

		string city_name(*token++);
		string area_code(*token++);
		string start_number(*token++);
		string end_number(*token++);
		string new_area_code(*token++);

		vector<string> calling_plan_names;
		vector<string> calling_plan_priorities;
		while(token != input_tokens.end()) {
			calling_plan_names.push_back(*token++);
			if(token == input_tokens.end()) error("incomplete input: line: ", line_no);
			calling_plan_priorities.push_back(*token != "" ? *token : "0");
			token++;
		}

		if(end_number == "") {
			int n = number_len - (area_code.length() + start_number.length());
			if(n < 0) error("number length error: line", line_no);
			end_number = start_number;
			for(; n > 0; --n) {
				start_number.append("0");
				end_number.append("9");
			}
		}

		vector<string>::iterator calling_plan_name = calling_plan_names.begin();
		vector<string>::iterator calling_plan_priority = calling_plan_priorities.begin();

		DEBUG(cerr << "city name: \"" << city_name << "\"" << endl;)
		DEBUG(cerr << "area code:    \"" << area_code << "\"" << endl;)
		DEBUG(cerr << "start number: \"" << start_number << "\"" << endl;)
		DEBUG(cerr << "end number:   \"" << end_number << "\"" << endl;)
		DEBUG(cerr << "new area code \"" << new_area_code << "\"" << endl;)
		DEBUG(cerr << "calling plan name(s):" << endl;)
		DEBUG(while(calling_plan_name != calling_plan_names.end()) \
					cerr << "\"" << *calling_plan_name++ << "\" prioirty: \"" << *calling_plan_priority++ << "\"" << endl;)

		calling_plan_name = calling_plan_names.begin();
		calling_plan_priority = calling_plan_priorities.begin();

		while(calling_plan_name != calling_plan_names.end()) {
			pair<set<CallingPlan*, CallingPlan_less>::iterator, bool> cp;

			if(new_area_code != "") {
				if(two_routes) {
					cp = calling_plans.insert(new CallingPlan(*calling_plan_name + (r1 == ::src ? "-s" : "-d")));

					(*cp.first)->addNumberRange(r1, city_name, *calling_plan_priority, area_code, start_number, end_number, new_area_code);

					cp = calling_plans.insert(new CallingPlan(*calling_plan_name + (r2 == ::src ? "-s" : "-d")));

					(*cp.first)->addNumberRange(r2, city_name, *calling_plan_priority, new_area_code, start_number, end_number, new_area_code);
				} else {
					cp = calling_plans.insert(new CallingPlan(*calling_plan_name + (r1 == ::src ? "-s" : "-d")));

					(*cp.first)->addNumberRange(r1, city_name, *calling_plan_priority, area_code, start_number, end_number, new_area_code);
				}
			} else {
				cp = calling_plans.insert(new CallingPlan(*calling_plan_name + (r1 == ::src ? "-s" : "-d")));

				(*cp.first)->addNumberRange(r1, city_name, *calling_plan_priority, area_code, start_number, end_number, area_code);
			}

			++calling_plan_name;
			++calling_plan_priority;

			showProgress();

		}
	}

	// Genenerate Routes
	cerr << "\bCreating Routes" << endl;
	for_each(calling_plans.begin(), calling_plans.end(), generateRoutes);

	// Calculate number of number ranges created
	total_number_ranges = 0;
	for_each(calling_plans.begin(), calling_plans.end(), calcNumberRanges);

	// Output stats
	cerr << "\bCreated " << total_number_ranges << " Number Range(s), ";
	cerr << calling_plans.size() << " Calling Plan(s), ";
	cerr << routes.size() << " Route(s), ";
	cerr << calling_plan_bindings.size() << " Calling Plan Binding(s)." << endl;

	cerr << "\bCreating XML file" << endl;
	cout << "<DB>" << endl << endl;

	// Generate the XML descricptions for the Calling Plans
	cerr << "\bCreating XML for Calling Plans" << endl;
	cout << "<!-- Start Calling Plans (# = " << calling_plans.size() << ") -->" << endl << endl;
	for_each(calling_plans.begin(), calling_plans.end(), emitCallingPlan);

	// Generate the XML descricptions for the Routes
	cerr << "\bCreating XML for Routes" << endl;
	cout << "<!-- Start Routes (# = " << routes.size() << ") -->" << endl << endl;
	for_each(routes.begin(), routes.end(), emitRoute);

	// Generate the XML descricptions for the Calling Plan to Routes bindings
	cerr << "\bCreating XML for Calling Plan Bindings" << endl;
	cout << "<!-- Start Calling Plan Bindings (# = " << calling_plan_bindings.size() << ") -->" << endl << endl;
	for_each(calling_plan_bindings.begin(), calling_plan_bindings.end(), emitCallingPlanBindings);

	cerr << "\bFinished" << endl;
	cout << "</DB>" << endl;

	cout.flush();

	cout.rdbuf(cout_sbuf);

	fout.close();
}
