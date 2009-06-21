// Cuboid.cpp
// Copyright (c) 2009, Dan Heeks
// This program is released under the BSD license. See the file COPYING for details.

#include "stdafx.h"
#include "Cuboid.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include "../interface/PropertyVertex.h"
#include "../interface/PropertyDouble.h"
#include "../interface/PropertyLength.h"
#include "Gripper.h"
#include "MarkedList.h"
#include "../tinyxml/tinyxml.h"

CCuboid::CCuboid(const gp_Ax2& pos, double x, double y, double z, const wxChar* title, const HeeksColor& col):CSolid(BRepPrimAPI_MakeBox(pos, x, y, z), title, col), m_pos(pos), m_x(x), m_y(y), m_z(z)
{
}

CCuboid::CCuboid(const TopoDS_Solid &solid, const wxChar* title, const HeeksColor& col):CSolid(solid, title, col), m_pos(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)), m_x(0.0), m_y(0.0), m_z(0.0)
{
}

HeeksObj *CCuboid::MakeACopy(void)const
{
	return new CCuboid(*this);
}

static void on_set_centre(const double *vt, HeeksObj* object){
	gp_Trsf mat;
	mat.SetTranslation ( gp_Vec ( ((CCuboid*)object)->m_pos.Location(), make_point(vt) ) );
	((CCuboid*)object)->m_pos.Transform(mat);
}

static void on_set_x(double value, HeeksObj* object){
	((CCuboid*)object)->m_x = value;
}

static void on_set_y(double value, HeeksObj* object){
	((CCuboid*)object)->m_y = value;
}

static void on_set_z(double value, HeeksObj* object){
	((CCuboid*)object)->m_z = value;
}

bool CCuboid::ModifyByMatrix(const double* m){
	gp_Trsf mat = make_matrix(m);
	gp_Ax2 new_pos = m_pos.Transformed(mat);
	double scale = gp_Vec(1, 0, 0).Transformed(mat).Magnitude();
	double new_x = fabs(m_x * scale);
	double new_y = fabs(m_y * scale);
	double new_z = fabs(m_z * scale);
	CCuboid* new_object = new CCuboid(new_pos, new_x, new_y, new_z, m_title.c_str(), m_color);
	new_object->CopyIDsFrom(this);
	wxGetApp().AddUndoably(new_object, m_owner, NULL);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().DeleteUndoably(this);
	return true;
}

void CCuboid::GetProperties(std::list<Property *> *list)
{
	double pos[3];
	extract(m_pos.Location(), pos);
	list->push_back(new PropertyVertex(_("datum corner"), pos, this, on_set_centre));
	list->push_back(new PropertyLength(_("width ( x )"), m_x, this, on_set_x));
	list->push_back(new PropertyLength(_("height( y )"), m_y, this, on_set_y));
	list->push_back(new PropertyLength(_("depth ( z )"), m_z, this, on_set_z));

	CSolid::GetProperties(list);
}

void CCuboid::GetGripperPositions(std::list<GripData> *list, bool just_for_endof)
{
	gp_Pnt o = m_pos.Location();
	gp_Pnt px(o.XYZ() + m_pos.XDirection().XYZ() * m_x);
	gp_Pnt py(o.XYZ() + m_pos.YDirection().XYZ() * m_y);
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt pz(o.XYZ() + z_dir.XYZ() * m_z);
	gp_Pnt m2(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y/2);
	gp_Pnt m3(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2 + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt m8(o.XYZ() + m_pos.YDirection().XYZ() * m_y/2 + z_dir.XYZ() * m_z);
	gp_Pnt pxy(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt pxz(o.XYZ() + m_pos.XDirection().XYZ() * m_x + z_dir.XYZ() * m_z);
	gp_Pnt pyz(o.XYZ() + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z);
	gp_Pnt pxyz(o.XYZ() + m_pos.XDirection().XYZ() * m_x  + m_pos.YDirection().XYZ() * m_y + z_dir.XYZ() * m_z);
	list->push_back(GripData(GripperTypeTranslate,o.X(),o.Y(),o.Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,px.X(),px.Y(),px.Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,py.X(),py.Y(),py.Z(),NULL));
	list->push_back(GripData(GripperTypeRotateObject,pz.X(),pz.Y(),pz.Z(),NULL));
	list->push_back(GripData(GripperTypeScale,pxyz.X(),pxyz.Y(),pxyz.Z(),NULL));
	list->push_back(GripData(GripperTypeRotate,pxy.X(),pxy.Y(),pxy.Z(),NULL));
	list->push_back(GripData(GripperTypeRotate,pxz.X(),pxz.Y(),pxz.Z(),NULL));
	list->push_back(GripData(GripperTypeRotate,pyz.X(),pyz.Y(),pyz.Z(),NULL));
	list->push_back(GripData(GripperTypeObjectScaleX,m2.X(),m2.Y(),m2.Z(),NULL));
	list->push_back(GripData(GripperTypeObjectScaleY,m3.X(),m3.Y(),m3.Z(),NULL));
	list->push_back(GripData(GripperTypeObjectScaleZ,m8.X(),m8.Y(),m8.Z(),NULL));
}

void CCuboid::OnApplyProperties()
{
	CCuboid* new_object = new CCuboid(m_pos, m_x, m_y, m_z, m_title.c_str(), m_color);
	new_object->CopyIDsFrom(this);
	wxGetApp().StartHistory();
	wxGetApp().AddUndoably(new_object, NULL, NULL);
	wxGetApp().DeleteUndoably(this);
	wxGetApp().EndHistory();
	wxGetApp().m_marked_list->Clear(true);
	if(wxGetApp().m_marked_list->ObjectMarked(this))wxGetApp().m_marked_list->Add(new_object, true);
	wxGetApp().Repaint();
}


bool CCuboid::GetScaleAboutMatrix(double *m)
{
	gp_Trsf mat = make_matrix(m_pos.Location(), m_pos.XDirection(), m_pos.YDirection());
	extract(mat, m);
	return true;
}

bool CCuboid::Stretch(const double *p, const double* shift, void* data)
{
	gp_Pnt vp = make_point(p);
	gp_Vec vshift = make_vector(shift);

	gp_Pnt o = m_pos.Location();
	gp_Dir z_dir = m_pos.XDirection() ^ m_pos.YDirection();
	gp_Pnt m2(o.XYZ() + m_pos.XDirection().XYZ() * m_x + m_pos.YDirection().XYZ() * m_y/2);
	gp_Pnt m3(o.XYZ() + m_pos.XDirection().XYZ() * m_x/2 + m_pos.YDirection().XYZ() * m_y);
	gp_Pnt m8(o.XYZ() + m_pos.YDirection().XYZ() * m_y/2 + z_dir.XYZ() * m_z);

	bool make_a_new_cuboid = false;

	if(m2.IsEqual(vp, wxGetApp().m_geom_tol)){
		m2 = m2.XYZ() + vshift.XYZ();
		double new_x = gp_Vec(m2.XYZ()) * gp_Vec(m_pos.XDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.XDirection());
		if(new_x > 0){
			make_a_new_cuboid = true;
			m_x = new_x;
		}
	}
	else if(m3.IsEqual(vp, wxGetApp().m_geom_tol)){
		m3 = m3.XYZ() + vshift.XYZ();
		double new_y = gp_Vec(m3.XYZ()) * gp_Vec(m_pos.YDirection()) - gp_Vec(o.XYZ()) * gp_Vec(m_pos.YDirection());
		if(new_y > 0){
			make_a_new_cuboid = true;
			m_y = new_y;
		}
	}
	else if(m8.IsEqual(vp, wxGetApp().m_geom_tol)){
		m8 = m8.XYZ() + vshift.XYZ();
		double new_z = gp_Vec(m8.XYZ()) * gp_Vec(z_dir) - gp_Vec(o.XYZ()) * gp_Vec(z_dir);
		if(new_z > 0){
			make_a_new_cuboid = true;
			m_z = new_z;
		}
	}

	if(make_a_new_cuboid)
	{
		CCuboid* new_object = new CCuboid(m_pos, m_x, m_y, m_z, m_title.c_str(), m_color);
		new_object->CopyIDsFrom(this);
		wxGetApp().StartHistory();
		wxGetApp().AddUndoably(new_object, NULL, NULL);
		wxGetApp().DeleteUndoably(this);
		wxGetApp().EndHistory();
		wxGetApp().m_marked_list->Clear(true);
		wxGetApp().m_marked_list->Add(new_object, true);
	}

	return true;
}

void CCuboid::SetXMLElement(TiXmlElement* element)
{
	const gp_Pnt& l = m_pos.Location();
	element->SetDoubleAttribute("lx", l.X());
	element->SetDoubleAttribute("ly", l.Y());
	element->SetDoubleAttribute("lz", l.Z());

	const gp_Dir& d = m_pos.Direction();
	element->SetDoubleAttribute("dx", d.X());
	element->SetDoubleAttribute("dy", d.Y());
	element->SetDoubleAttribute("dz", d.Z());

	const gp_Dir& x = m_pos.XDirection();
	element->SetDoubleAttribute("xx", x.X());
	element->SetDoubleAttribute("xy", x.Y());
	element->SetDoubleAttribute("xz", x.Z());

	element->SetDoubleAttribute("wx", m_x);
	element->SetDoubleAttribute("wy", m_y);
	element->SetDoubleAttribute("wz", m_z);

	CSolid::SetXMLElement(element);
}

void CCuboid::SetFromXMLElement(TiXmlElement* pElem)
{
	double l[3] = {0, 0, 0};
	double d[3] = {0, 0, 1};
	double x[3] = {1, 0, 0};

	for(TiXmlAttribute* a = pElem->FirstAttribute(); a; a = a->Next())
	{
		std::string name(a->Name());
		if(name == "lx")	 {l[0] = a->DoubleValue();}
		else if(name == "ly"){l[1] = a->DoubleValue();}
		else if(name == "lz"){l[2] = a->DoubleValue();}

		else if(name == "dx"){d[0] = a->DoubleValue();}
		else if(name == "dy"){d[1] = a->DoubleValue();}
		else if(name == "dz"){d[2] = a->DoubleValue();}

		else if(name == "xx"){x[0] = a->DoubleValue();}
		else if(name == "xy"){x[1] = a->DoubleValue();}
		else if(name == "xz"){x[2] = a->DoubleValue();}

		else if(name == "wx"){m_x = a->DoubleValue();}
		else if(name == "wy"){m_y = a->DoubleValue();}
		else if(name == "wz"){m_z = a->DoubleValue();}
	}

	m_pos = gp_Ax2(make_point(l), make_vector(d), make_vector(x));

	CSolid::SetFromXMLElement(pElem);
}