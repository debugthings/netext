/*=========================================================================================================
  Copyright (c) 2013-2015 Rodney Viana
  http://netext.codeplex.com/

  Distributed under GNU General Public License version 2 (GPLv2) (http://www.gnu.org/licenses/gpl-2.0.html)
  ============================================================================================================*/

#include "NetExt.h"

EXT_COMMAND(wclrstack,
	"Dump current stack trace",
	"{contains;s,o;;contains;List of field types that the type must contain (eg. -contains String, Object)}"
	"{;e,o,d=@$peb;peb;PEB address}")
{
	DO_INIT_API;
	using namespace NetExtShim;

	bool hasArg = HasArg("contains");
	wstring argString;
	if (hasArg)
	{
		argString.assign(CA2W(GetArgStr("contains")));
	}

	CComPtr<IMDThreadEnum>  threadEnum;

	HRESULT hr = pRuntime->EnumerateThreads(&threadEnum);
	EXITPOINT("Unable to enumerate threads");

	ULONG threadId;
	ULONG sysId;
	g_ExtInstancePtr->m_System3->GetCurrentThreadSystemId(&sysId);
	g_ExtInstancePtr->m_System3->GetCurrentThreadId(&threadId);
	while (hr == S_OK)
	{
		CComPtr<IMDThread> thread;
		hr = threadEnum->Next(&thread);

		long osId;

		if (hr == S_OK && thread->GetOSThreadId(&osId) == S_OK)
		{

			if ((ULONG)osId == sysId)
			{
				long isAlive;
				thread->IsAlive(&isAlive);
				long lockCount;
				thread->GetLockCount(&lockCount);
				long isFinalizer;
				thread->IsFinalizer(&isFinalizer);

				CComPtr<IMDStackTraceEnum> stackEnum;
				Out("Thread Id: %i OS Id: %x Locks: %i\n", threadId, sysId, lockCount);
				if (isAlive)
					Out("Thread is Alive\n");
				else
					Out("Thread IS NOT Alive\n");
				if (isFinalizer)
					Out("Finalizer thread\n");

				CComPtr<IMDException> ex;

				thread->GetCurrentException(&ex);
				if (ex)
				{
					CComBSTR exString;
					ex->GetErrorMessage(&exString);
					CComPtr<IMDType> clrType;
					ex->GetGCHeapType(&clrType);
					Out("Last Exception: ");
					if (clrType)
					{
						CComBSTR typeName;
						clrType->GetName(&typeName);
						Out("(%S) ", typeName);
					}
					Out("%S\n", exString);
				}

				if (thread->EnumerateStackTrace(&stackEnum) == S_OK)
				{
					CLRDATA_ADDRESS ip;
					CLRDATA_ADDRESS sp;
					CComBSTR frameStr;
					wstring compareString;

					Out("\n");

					while (stackEnum->Next(&ip, &sp, &frameStr) == S_OK)
					{
						compareString.assign(frameStr);
						if (hasArg)
							if (!g_ExtInstancePtr->MatchPattern(CW2A(compareString.c_str()), CW2A(argString.c_str())))
								continue;
						Out("%p %p %S\n", sp, ip, frameStr);


					}
				}
				return;
			}
		}


	}

	//pHeap->


}