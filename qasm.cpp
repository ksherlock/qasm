#include "app.h"
#include "asm.h"
#ifdef CIDERPRESS
#include "cider.h"
#endif

#define CLASS PAL_APPCLASS

// return a pointer to the actual Application class
PAL_BASEAPP *PAL::appFactory(void)
{
	return (new CLASS());
}

// you MUST supply this array 'appOptions'.  NULL line and end.
programOption PAL::appOptions[] =
{
#ifdef DEBUG
	{ "debug", "d", "enable debug info (repeat for more verbosity)", "", false, true},
#endif
	//{ "config", "f", "load configuration data from a <file>", "<file>", false, false},
	{ "exec", "x", "execute a command [asm, link, format, script] default=asm", "<command>", false, false},
	{ "objfile", "o", "write output to <objfile>", "<objfile>", false, false},
	{ "instruction", "i", "force the CPU instruction set ('xc' ingored) [6502, 65C02, 65816]", "<cpu>", false, false},

	{ "type", "t", "enforce syntax/features/bugs of other assembler [qasm, merlin8, merlin16, merlin16plus, merlin32, orca, apw, mpw, lisa, ca65]", "<type>", false, false},
	{ "quiet", "q", "print as little as possible, equivalent to lst off in the assembler ('lst' opcodes will be ignored)", "", false, true},
	{ "list", "l", "force assembly listing ('lst' opcodes will be ignored)", "", false, true},

	{ "color", "c", "colorize the output", "", false, true},
	{ "parms", "p", "show the working parameters/options", "", false, true},



	{ "", "", "", "", false, false}
};


void CLASS::displayVersion()
{
	std::string s = "";
#ifdef DEBUG
	s = "-debug";
#endif
	cerr << "quickASM 16++ v" << (std::string)STRINGIFY(APPVERSION) << s << endl;

//#ifdef CIDERPRESS
//	DiskImgLib::Global::AppInit();
//	DiskImgLib::DiskImg prodos;

//   DiskImgLib::Global::AppCleanup();
//#endif

}

void CLASS::showerror(int ecode, std::string fname)
{
	std::string s;
	switch (ecode)
	{
	case -2:
		s = "Permission Denied";
		break;
	case -3:
		s = "File not found";
		break;
	default:
		s = "Unknown Error";
		break;
	}
	if (ecode < -1)
	{
		std::string a = Poco::Util::Application::instance().config().getString("application.name", "");
		fprintf(stderr, "%s: %s: %s\n", a.c_str(), fname.c_str(), s.c_str());
	}
}

// int main(int argc, char *argv[])
// this is where libpal calls to run a command line program
int CLASS::runCommandLineApp(void)
{
	TFileProcessor *t = NULL;
	std::string line;
	std::string startdirectory;
	std::string appPath;
	std::string fname;
	//uint32_t syntax;
	string product="";
	string syn;
	int res = -1;
	ConfigOptions options;

//Poco::Util::Application::instance().config()

	utils=new QUtils();

	//LOG_NOTE << "this is it!" << endl;

	startdirectory = Poco::Path::current();
	appPath=utils->getAppPath();
	if (commandargs.size() == 0)
	{
		displayHelp();
		return (res);
	}

	options.ReadFile(Poco::Path::config()+"/parms.json");
	options.ReadFile(appPath+"/parms.json");
	options.ReadFile(Poco::Path::configHome()+"/parms.json");
	options.ReadFile(Poco::Path::current()+"/parms.json");

	syn="QASM";
	//syn=options.GetString("assembler.syntax","QASM");

	string cmdsyn = Poco::toUpper(getConfig("option.type", ""));
	if (cmdsyn!="")
	{
		syn=cmdsyn; // if they overrode the syntax on the command line, use it
	}

	syn=Poco::toUpper(syn);
	product=Poco::trim(syn);

	if (isDebug()>0)
	{
		printf("SYNTAX: |%s|\n",syn.c_str());
	}

	try
	{
#ifdef CIDERPRESS

		//CiderPress *cp=new CiderPress();
		//int err=cp->CreateVolume("./a2.2mg","PRODOS1",800*1024,CP_PRODOS);
		//printf("volume create: %d\n",err);

		//cp->RunScript(startdirectory+"/disk_commands.txt");

		//delete(cp);

#endif

		for (ArgVec::const_iterator it = commandargs.begin(); it != commandargs.end(); ++it)
		{
			int32_t format_flags=CONVERT_LINUX;
			Poco::File fn(*it);
			int x;
			std::string p = fn.path();
			Poco::Path path(p);
			//logger().information(path.toString());

			std::string e = toUpper(path.getExtension());

			std::string cmd = Poco::toUpper(getConfig("option.exec", "asm"));

			Poco::StringTokenizer toks(cmd,"-");
			if (toks.count()>1)
			{
				if (toks[0]=="FORMAT")
				{
					if (toks[1]=="LINUX")
					{
						format_flags=CONVERT_LINUX;
					}
					else if (toks[1]=="WINDOWS")
					{
						format_flags=CONVERT_WINDOWS;
					}
					else if (toks[1]=="MERLIN")
					{
						format_flags=CONVERT_MERLIN;
					}
					else if (toks[1]=="APW")
					{
						format_flags=CONVERT_APW;
					}
					else if (toks[1]=="MPW")
					{
						format_flags=CONVERT_MPW;
					}
					else if (toks[1]=="MAC")
					{
						format_flags=CONVERT_LINUX;
					}
					else if (toks[1]=="CC65")
					{
						format_flags=CONVERT_LINUX;
					}
					else if (toks[1]=="COMPRESS")
					{
						format_flags=CONVERT_TEST;
					}
					options.format_flags=format_flags;
					cmd=toks[0];
				}
			}
			if (cmd.length() > 0)
			{
				if (cmd == "FORMAT")
				{
					res = 0;
					t = new TMerlinConverter(options);
					//t=NULL;
					if (t != NULL)
					{
						try
						{
							t->init();
							t->setProduct(product);
							t->format_flags=format_flags;

							std::string f = path.toString();
							t->filename = f;
							x = t->processfile(f, fname);
							if (x == 0)
							{
								t->process();
								t->complete();
							}
							else
							{
								showerror(x, fname);
								t->errorct = 1;
							}
							res = (t->errorct > 0) ? -1 : 0;
						}
						catch (...)
						{
							delete t;
							t = NULL;
						}
					}
				}
				else if (cmd == "ASM")
				{
					int x;
					t = new T65816Asm(options);
					if (t != NULL)
					{
						try
						{
							t->init();
							t->setProduct(product);


							std::string f = path.toString();
							t->filename = f;
							x = t->processfile(f, fname);
							f = t->filename;
							if (x == 0)
							{
								t->process();
								t->complete();
							}
							else
							{
								showerror(x, fname);
								t->errorct = 1;
							}
							res = (t->errorct > 0) ? -1 : 0;
						}
						catch(const std::exception& e)
						{
							delete t;
							t = NULL;
						}
						if (chdir(startdirectory.c_str())) {}; // return us back to where we were
					}
				}
#ifdef CIDERPRESS
				else if (cmd == "SCRIPT")
				{
					res = 0;
					t = new CiderPress(options);
					if (t!=NULL)
					{
						try
						{
							t->init();
							t->setProduct(product);

							std::string f = path.toString();
							t->filename = f;
							x = t->processfile(f, fname);
							f = t->filename;
							if (x == 0)
							{
								t->process();
								t->complete();
							}
							else
							{
								showerror(x, fname);
								t->errorct = 1;
							}
							res = (t->errorct > 0) ? -1 : 0;
						}
						catch(const std::exception& e)
						{
							std::cerr << e.what() << '\n';
							if (t!=NULL)
							{
								delete t;
								t=NULL;
							}
						}
					}
				}
#endif
				else
				{
					printf("not supported type\n");
				}
			}
			else
			{
				fprintf(stderr, "Invalid command: <%s>\n\n", cmd.c_str());
			}
		}
	}
	catch(...)
	{
#ifdef CIDERPRESS
		DiskImgLib::Global::AppCleanup();
#endif
	}
	return (res);
}



