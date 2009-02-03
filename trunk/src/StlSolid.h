// StlSolid.h

#include "../interface/HeeksObj.h"

class CStlTri{
public:
	float n[3];
	float x[3][3];
};

class CStlSolid:public HeeksObj{
private:
	static wxIcon* m_icon;
	HeeksColor color;
	int m_gl_list;
	CBox m_box;
	wxString m_title;

	void read_from_file(const wxChar* filepath);

public:
	std::list<CStlTri> m_list;

	CStlSolid(const HeeksColor* col);
	CStlSolid(const wxChar* filepath, const HeeksColor* col);
	~CStlSolid();

	virtual const CStlSolid& operator=(const CStlSolid& s);

	int GetType()const{return StlSolidType;}
	long GetMarkingMask()const{return MARKING_FILTER_STL_SOLID;}
	int GetIDGroupType()const{return SolidType;}
	const wxChar* GetTypeString(void)const{return _("STL Solid");}
	wxString GetIcon(){return _T("stlsolid");}
	void glCommands(bool select, bool marked, bool no_color);
	void GetBox(CBox &box);
	void KillGLLists(void);
	bool ModifyByMatrix(const double* m);
	const wxChar* GetShortString(void)const{return m_title.c_str();}
	bool CanEditString(void)const{return true;}
	void OnEditString(const wxChar* str);
	void GetTriangles(void(*callbackfunc)(const double* x, const double* n), double cusp, bool just_one_average_normal = true);
	void CopyFrom(const HeeksObj* object);
	HeeksObj *MakeACopy()const;
	void WriteXML(TiXmlNode *root);

	static HeeksObj* ReadFromXMLElement(TiXmlElement* pElem);
};

