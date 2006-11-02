
#pragma once

#include <atlddx.h>

#define DDX_EX_FLOAT_POSITIVE(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Numeric_Inf<float>(nID, var, bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_EX_INT_POSITIVE_LIMIT(nID, var, lower, include) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Numeric_Inf<int>(nID, var, bSaveAndValidate, lower, include)) \
				return FALSE; \
		}

#define DDX_EX_INT_POSITIVE(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Numeric_Inf<int>(nID, var, bSaveAndValidate)) \
				return FALSE; \
		}

template <class T>
class CWinDataExchangeEx : public CWinDataExchange<T>
{
public:
	template <typename N>
	BOOL DDX_Numeric_Inf(UINT nID, N& nVal, BOOL bSave, N lower_limit = 0, bool include_limit = true, BOOL bValidate = FALSE)
	{
		T* pT = static_cast<T*>(this);
		bool bSuccess = true;
		const int cchBuff = 128;
		wchar_t szBuff[cchBuff] = { 0 };
		
		if(bSave)
		{
			pT->GetDlgItemText(nID, szBuff, cchBuff);
			try
			{
			
			nVal = lexical_cast<N>(szBuff);
			if (include_limit)
				if (nVal < lower_limit) nVal = -1;
			else
				if (nVal <= lower_limit) nVal = -1;		
			
			}        
			catch(boost::bad_lexical_cast &)
			{
			nVal = -1;
			}
			
			if (nVal < 0)
				pT->SetDlgItemText(nID, L"∞");
		}
		else
		{
			wstring number = L"∞";
			
			if (include_limit)
				if (nVal >= lower_limit) number = lexical_cast<wstring>(nVal);
			else
				if (nVal > lower_limit) number = lexical_cast<wstring>(nVal);
			
			pT->SetDlgItemText(nID, number.c_str());
		}
		
		return bSuccess;
	} 
};
