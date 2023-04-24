/**
 * @file TCell.h
 * @class TCell, Cell, ...
 * @date 15 Mar 2021
 * @ingroup umts_rlc
 * @brief
 */


#ifndef TCELL_H
#define TCELL_H

#include <iostream>
#include <map>
#include <memory>
using namespace std;

enum class MemProtection
{
	PROTECTED,
	NON_PROTECTED
};

template< typename T >
class TCell;

class Cell
{
public:

	Cell(const std::string& name, MemProtection dataProtected) //note: we don't lightly copy strings in C++
		: name(name) , dataMemProtected(dataProtected){}
	virtual ~Cell() {
		
	}	
	string CellType;

	template < typename T >
	TCell<T>* cast()
	{		
		return (TCell<T>*)(this);		
	}
	string getName() {
		return name;
	}
	int getLength() { return length; }
	MemProtection getProtection()
	{
		return dataMemProtected;
	}
protected:
	std::string name;
	int length=0;
	MemProtection dataMemProtected= MemProtection::NON_PROTECTED;
	
};

template< typename T >
class TCell : public Cell
{
public:
	TCell(const std::string& name, T const& dataVal, MemProtection dataProtected) : Cell(name, dataProtected)
	{
		
		data = dataVal;
		length = 1;
		CellType = typeid(T).name();

	}
	TCell(const std::string& name, const T& ptr, int Length, MemProtection dataProtected) : Cell(name, dataProtected)
	{
		data = ptr;
		length = Length;
		CellType = typeid(T).name();
	
	}
	TCell(const Cell &x) : Cell(x)
	{
		
	}

	void setData(T value) {
		data = value;
	}
	~TCell(){
		length = 0;
		name = "";
		CellType = "";
	}
	T getData(){
		return data;
	}

private:
	T data;
};

class Pos
{
public:
	Pos(int x, int y) : X(x), Y(y) {}
	int X;
	int Y;
	inline bool operator<(const Pos& PosObj) const
	{
		if (this->X < PosObj.X) {
			return true;
		}
		else if (this->X > PosObj.X)
		{
			return false;
		}
		else
		{
			if (this->Y < PosObj.Y)
			{
				return true;
			}
			else if (this->Y > PosObj.Y)
			{
				return false;
			}
		}
		return false;
	}
};

class CellList
{
public:

	CellList()
	{
		// do not alloc memory here
		// because when we create TCell<CellList>, Tcell create one CellList because Tcell.data type's is CellList
		// so CellList Constructor will be called and redundant memory will be allocated.
	}
	CellList(string listName)
	{
		listname = listName;
	}
	template<typename T1>
	void add(string name, T1 const& data, MemProtection dataMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>(name, data, dataMemProtected);

		Pos nextPos = getNextRowPos();

		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));

	}
	template<typename T1>
	void addPtr(string name, T1 ptr, int len, MemProtection ptrMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>(name, ptr, len, ptrMemProtected);
		Pos nextPos = getNextRowPos();
		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));

	}
	template<typename T1>
	void add(T1 const& data, MemProtection dataMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>("", data, dataMemProtected);
		Pos nextPos = getNextRowPos();
		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));
	}
	template<typename T1>
	void addPtr(T1 ptr, int len, MemProtection ptrMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>("", ptr, len, ptrMemProtected);
		Pos nextPos = getNextRowPos();
		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));
	}

	void addCell(Cell* cell)
	{
		checkAndAllocMem();

		Pos nextPos = getNextRowPos();

		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));

	}

	template<typename T1>
	TCell<T1>* getCell(int index)
	{
		Pos pos = Pos(index, 0);
		bool keyexist = exist(pos);
		if (keyexist)
		{
			return at(pos)->cast<T1>();
		}
		return NULL;
	}

	template<typename T1>
	TCell<T1>* getCell(string name)
	{
		Pos cellPos = getPos(name);
		if (cellPos.X > -1)
		{
			return at(cellPos)->cast<T1>();
		}
		else
		{
			int index = 0;
			string searchName = "";
			do {
				index = name.find_first_of('.');

				if (index >= 0)
				{
					string header = name.substr(0, index);
					string tail = name.substr(index + 1);
					if (searchName.length() == 0)
					{
						searchName = header;
					}
					else
					{
						searchName = searchName + "." + header;
					}
					//search for head here, if exist search for tail there
					Pos headPos = getPos(searchName);
					if (headPos.X > -1)
					{
						// find head
						if (at(headPos)->CellType == typeid(CellList).name())
						{
							auto nestedlist = at(headPos)->cast<CellList>()->getData();
							auto res = nestedlist.getCell<T1>(tail);
							return res;
						}
						else
						{
							return NULL;
						}
					}
					else
					{
						name = tail;
						continue;
					}
				}
			} while (index > 0);
		}

		return NULL;
	}

	template<typename T1>
	T1 getValue(int index)
	{
		return getCell<T1>(index, 0)->getData();
	}

	template<typename T1>
	T1 getValue(string name)
	{
		auto res = getCell<T1>(name);
		if (res)
		{
			return res->getData();
		}
		return T1();
	}

	template<typename T1>
	T1 getValueAt(int index,string name)
	{
		auto tcell = getCell<CellList>(index);
		if (tcell)
		{
			T1 val = tcell->getData().getValue<T1>(name);
			return val;
		}
		return T1();


	}

	template<typename T1>
	void setValue(int index, T1 value)
	{
		return getCell<T1>(index, 0)->setData(value);
	}

	template<typename T1>
	void setValue(string name, T1 value)
	{
		auto res = getCell<T1>(name);
		if (res)
		{
			res->setData(value);
			return;
		}
		return ;
	}

	template<typename T1>
	void setValueAt(int index, string name, T1 value)
	{
		auto tcell = getCell<CellList>(index);
		if (tcell)
		{
			tcell->getData().setValue<T1>(name,value);
		}
		return ;
	}

	bool contain(string name)
	{
		/*for (int i = 0; i < size(); i++)
		{
			string fullname = at(i)->getName();
			if (fullname == name)
			{
				return i;
			}
		}*/
		map<Pos, Cell*>::iterator it;

		for (it = listSharedPtr->begin(); it != listSharedPtr->end(); it++)
		{
			//std::cout << it->first    // string (key)
			if (it->second->getName() == name)
			{
				return true;
			}
		}
		return false;
	}

	void clear(bool releaseCellData = true)
	{
		if (listSharedPtr == NULL)
		{
			return;
		}
		int count = size();
		for (int i = 0; i < count; i++)
		{
			removeAt(listSharedPtr->begin()->first.X, releaseCellData);
		}
		releaseRoot();
	}

	inline void removeAt(int index, bool releaseCellData );

	int size()
	{
		auto ptr = listSharedPtr.get();
		if (ptr == NULL)
		{
			return 0;
		}
		int len = listSharedPtr.get()->size();
		return len;
	}

	bool exist(int rowIndex)
	{
		Pos pos = Pos(rowIndex, 0);
		return exist(pos);
	}
	Pos getPos(string name)
	{
		map<Pos, Cell*>::iterator it;
		for (it = listSharedPtr->begin(); it != listSharedPtr->end(); it++)
		{
			//std::cout << it->first    // string (key)
			if (it->second->getName() == name)
			{
				return it->first;
			}
		}
		return Pos(-1, -1);
	}
	CellList(const CellList& x)
	{
		listSharedPtr = x.listSharedPtr;
		listname = x.listname;
	}
	//Cell* operator[](int index) {

	//	return at(index,0);
	//}
	CellList& operator=(const CellList& s)
	{
		listSharedPtr = s.listSharedPtr;
		listname = s.listname;
		return *this;
	}
	~CellList()
	{

		// we cant place delete listptr here in order to release listptr when CellList goes out of scope,
		// because when we create a 'TCell<CellList> A' then type-of(A.data=B) is CellList, now if we want to delete A, it will call member destructor
		// and may be internal CellList(B) deleted unintentionally. internal CellList(B) may be added in multiple CellList and we lost B in other its container.
	}
private:

	bool exist(Pos pos)
	{
		auto ptr = listSharedPtr.get();
		if (ptr == NULL)
		{
			return false;
		}
		int count = ptr->count(pos);
		if (count > 0)
		{
			return true;
		}
		return false;
	}
	Cell* at(Pos pos)
	{
		bool keyexist = exist(pos);
		if (keyexist)
		{
			return listSharedPtr.get()->at(pos);
		}
		return NULL;
	}
	void erase(Pos pos)
	{
		listSharedPtr.get()->erase(pos);
	}
	void releaseRoot()
	{
		/*if (listptr == NULL)
		{
			return;
		}
		delete this->listptr;
		listptr = NULL;*/

		listSharedPtr.reset();
	}
	Pos getNextRowPos()
	{
		Pos pos(0, 0);
		if (size() > 0)
		{
			pos = std::prev(listSharedPtr.get()->end())->first;
			pos = Pos(pos.X + 1, 0);
		}
		return pos;
	}
	void checkAndAllocMem()
	{
		if (listSharedPtr == NULL)
		{
			std::map<Pos, Cell*>* ptr = new std::map<Pos, Cell*>();
			listSharedPtr.reset(ptr);
		}
	}
	string listname;
	shared_ptr<std::map<Pos, Cell*>> listSharedPtr;
};
class CellMat
{
public:

	CellMat()
	{
		// do not alloc memory here
		// because when we create TCell<CellMat>, Tcell create one CellMat because Tcell.data type's is CellMat
		// so CellMat Constructor will be called and redundant memory will be allocated.
	}
	CellMat(string MatName)
	{
		matName = MatName;
	}
	template<typename T1>
	void addAt(int rowIndex,int colIndex,string name, T1 const& data, MemProtection dataMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>(name, data, dataMemProtected);

		Pos nextPos(rowIndex,colIndex);

		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));

	}
	template<typename T1>
	void addPtrAt(int rowIndex, int colIndex, string name, T1 ptr, int len, MemProtection ptrMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>(name, ptr, len, ptrMemProtected);
		Pos nextPos(rowIndex, colIndex);
		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));

	}
	template<typename T1>
	void addAt(int rowIndex, int colIndex, T1 const& data, MemProtection dataMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>("", data, dataMemProtected);
		Pos nextPos(rowIndex, colIndex);
		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));
	}
	template<typename T1>
	void addPtrAt(int rowIndex, int colIndex, T1 ptr, int len, MemProtection ptrMemProtected = MemProtection::NON_PROTECTED)
	{
		checkAndAllocMem();
		auto cell = new TCell<T1>("", ptr, len, ptrMemProtected);
		Pos nextPos(rowIndex, colIndex);
		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));
	}

	void addCellAt(int rowIndex, int colIndex,Cell* cell)
	{
		checkAndAllocMem();

		Pos nextPos(rowIndex, colIndex);

		listSharedPtr.get()->insert(std::make_pair(nextPos, cell));

	}

	template<typename T1>
	TCell<T1>* getCell(int rowIndex,int colIndex)
	{
		Pos pos = Pos(rowIndex, colIndex);
		bool keyexist = exist(pos);
		if (keyexist)
		{
			return at(pos)->cast<T1>();
		}
		return NULL;
	}

	template<typename T1>
	TCell<T1>* getCell(string name)
	{
		Pos cellPos = getPos(name);
		if (cellPos.X > -1)
		{
			return at(cellPos)->cast<T1>();
		}
		else
		{
			int index = 0;
			string searchName = "";
			do {
				index = name.find_first_of('.');

				if (index >= 0)
				{
					string header = name.substr(0, index);
					string tail = name.substr(index + 1);
					if (searchName.length() == 0)
					{
						searchName = header;
					}
					else
					{
						searchName = searchName + "." + header;
					}
					//search for head here, if exist search for tail there
					Pos headPos = getPos(searchName);
					if (headPos.X > -1)
					{
						// find head
						if (at(headPos)->CellType == typeid(CellMat).name())
						{
							auto nestedlist = at(headPos)->cast<CellMat>()->getData();
							auto res = nestedlist.getCell<T1>(tail);
							return res;
						}
						else
						{
							return NULL;
						}
					}
					else
					{
						name = tail;
						continue;
					}
				}
			} while (index > 0);
		}

		return NULL;
	}

	template<typename T1>
	T1 getValue(int rowIndex, int colIndex )
	{
		return getCell<T1>(rowIndex,colIndex)->getData();
	}

	template<typename T1>
	T1 getValue(string name)
	{
		auto res = getCell<T1>(name);
		if (res)
		{
			return res->getData();
		}
		return T1();
	}

	template<typename T1>
	T1 getValueAt(int rowindex, int colIndex,string name)
	{
		auto tcell = getCell<CellMat>(rowindex,colIndex);
		if (tcell)
		{	
			T1 val = tcell->getData().getValue<T1>(name);
			return val;
		}
		return T1();

		
	}

	template<typename T1>
	void setValue(int rowIndex, int colIndex,T1 value)
	{
		return getCell<T1>(rowIndex, colIndex)->setData(value);
	}

	template<typename T1>
	void setValue(string name, T1 value)
	{
		auto res = getCell<T1>(name);
		if (res)
		{
			res->setData(value);
		}
		return ;
	}

	template<typename T1>
	void setValueAt(int rowindex, int colIndex, string name, T1 value)
	{
		auto tcell = getCell<CellMat>(rowindex, colIndex);
		if (tcell)
		{
			tcell->getData().setValue<T1>(name,value);
		}
		return ;


	}


	bool contain(string name)
	{
		/*for (int i = 0; i < size(); i++)
		{
			string fullname = at(i)->getName();
			if (fullname == name)
			{
				return i;
			}
		}*/
		map<Pos, Cell*>::iterator it;

		for (it = listSharedPtr->begin(); it != listSharedPtr->end(); it++)
		{
			//std::cout << it->first    // string (key)
			if (it->second->getName() == name)
			{
				return true;
			}
		}
		return false;
	}

	void clear(bool releaseCellData = true)
	{
		if (listSharedPtr == NULL)
		{
			return;
		}
		map<Pos, Cell*>::iterator it;

		int count = size();
		for (int i = 0; i < count; i++)
		{
			removeAt(listSharedPtr->begin()->first.X, listSharedPtr->begin()->first.Y, releaseCellData);
		}
		releaseRoot();
	}
	inline void removeAt(int rowIndex, int colIndex , bool releaseCellData);

	int size()
	{
		auto ptr = listSharedPtr.get();
		if (ptr == NULL)
		{
			return 0;
		}
		int len = listSharedPtr.get()->size();
		return len;
	}

	bool exist(int rowIndex,int colIndex)
	{		
		Pos pos = Pos(rowIndex, colIndex);
		return exist(pos);	
	}
	Pos getPos(string name)
	{
		map<Pos, Cell*>::iterator it;
		for (it = listSharedPtr->begin(); it != listSharedPtr->end(); it++)
		{
			//std::cout << it->first    // string (key)
			if (it->second->getName() == name)
			{
				return it->first;
			}
		}
		return Pos(-1, -1);
	}
	CellMat(const CellMat& x)
	{
		listSharedPtr = x.listSharedPtr;
		matName = x.matName;
	}
	//Cell* operator[](int index) {

	//	return at(index,0);
	//}
	CellMat& operator=(const CellMat& s)
	{
		listSharedPtr = s.listSharedPtr;
		matName = s.matName;
		return *this;
	}
	~CellMat()
	{

		// we cant place delete listptr here in order to release listptr when CellMat goes out of scope,
		// because when we create a 'TCell<CellMat> A' then type-of(A.data=B) is CellMat, now if we want to delete A, it will call member destructor
		// and may be internal CellMat(B) deleted unintentionally. internal CellMat(B) may be added in multiple CellMat and we lost B in other its container.
	}
private:	
	
	bool exist(Pos pos)
	{
		auto ptr = listSharedPtr.get();
		if (ptr == NULL)
		{
			return false;
		}
		int count = ptr->count(pos);
		if (count > 0)
		{
			return true;
		}
		return false;
	}
	Cell* at(Pos pos)
	{
		bool keyexist = exist(pos);
		if (keyexist)
		{
			return listSharedPtr.get()->at(pos);
		}
		return NULL;
	}
	void erase(Pos pos)
	{
		listSharedPtr.get()->erase(pos);
	}
	void releaseRoot()
	{
		/*if (listptr == NULL)
		{
			return;
		}
		delete this->listptr;
		listptr = NULL;*/
		
		listSharedPtr.reset();
	}
	Pos getNextRowPos()
	{
		Pos pos(0, 0);
		if (size() > 0)
		{
			pos = std::prev(listSharedPtr.get()->end())->first;
			pos = Pos(pos.X + 1, 0);
		}
		return pos;
	}
	void checkAndAllocMem()
	{
		if (listSharedPtr == NULL)
		{
			std::map<Pos, Cell*>* ptr = new std::map<Pos, Cell*>();
			listSharedPtr.reset(ptr);
		}
	}
	string matName;
	shared_ptr<std::map<Pos,Cell*>> listSharedPtr;
};

void CellList::removeAt(int index, bool releaseCellData = true)
{
	Pos cellPos = Pos(index, 0);
	bool indExist = exist(cellPos);
	if (indExist)
	{
		if (releaseCellData)
		{
			string cellType = at(cellPos)->CellType;
			MemProtection memProtection = at(cellPos)->getProtection();
			int dataLength = at(cellPos)->getLength();
	
			if (dataLength > 1 && memProtection == MemProtection::NON_PROTECTED)
			{
				// if object type is array of custom class this mechanism for deleting pointer is invalid and we need cast
				//pointer to that custom type and then call delete. we cant cast custopm class pointer to  void* and then delete it.
				// so do not add array of custom class or handle it here 
				auto cell = at(cellPos)->cast<void*>();
				auto data = cell->getData();
				if (data != NULL)
				{
					delete[] data;
					data = NULL;
				}
			}
			if (cellType == typeid(CellList).name() && memProtection == MemProtection::NON_PROTECTED)
			{
				auto cell = getCell<CellList>(cellPos.X);
				CellList data = cell->getData();
				data.clear();
			}
			if (cellType == typeid(CellMat*).name() && memProtection == MemProtection::NON_PROTECTED)
			{
				auto cell = getCell<CellMat>(cellPos.X);
				CellMat data = cell->getData();
				data.clear();
			}
		}
		auto cell = at(cellPos);
		delete cell;
		cell = NULL;
		erase(cellPos);
	}
}
void CellMat::removeAt(int rowIndex, int colIndex , bool releaseCellData = true)
{
	Pos cellPos = Pos(rowIndex, colIndex);
	bool indExist = exist(cellPos);
	if (indExist)
	{
		if (releaseCellData)
		{
			string cellType = at(cellPos)->CellType;
			MemProtection memProtection = at(cellPos)->getProtection();
			int dataLength = at(cellPos)->getLength();

			if (dataLength > 1 && memProtection == MemProtection::NON_PROTECTED)
			{
				// if object type is array of custom class this mechanism for deleting pointer is invalid and we need cast
				//pointer to that custom type and then call delete. we cant cast custopm class pointer to  void* and then delete it.
				// so do not add array of custom class or handle it here 
				auto cell = at(cellPos)->cast<void*>();
				auto data = cell->getData();
				if (data != NULL)
				{
					delete[] data;
					data = NULL;
				}
			}
			if (cellType == typeid(CellList).name() && memProtection == MemProtection::NON_PROTECTED)
			{
				auto cell = getCell<CellList>(cellPos.X,cellPos.Y);
				CellList data = cell->getData();
				data.clear();
			}
			if (cellType == typeid(CellMat).name() && memProtection == MemProtection::NON_PROTECTED)
			{
				auto cell = getCell<CellMat>(cellPos.X, cellPos.Y);
				CellMat data = cell->getData();
				data.clear();
			}
		}
		auto cell = at(cellPos);
		delete cell;
		cell = NULL;
		erase(cellPos);
	}
}

#endif
