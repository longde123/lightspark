/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009-2011  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "Array.h"
#include "abc.h"
#include "argconv.h"
#include "parsing/amf3_generator.h"

using namespace std;
using namespace lightspark;

SET_NAMESPACE("");
REGISTER_CLASS_NAME(Array);

Array::Array()
{
	currentsize=0;
	type=T_ARRAY;
}

void Array::sinit(Class_base* c)
{
	c->setConstructor(Class<IFunction>::getFunction(_constructor));
	// public constants
	c->setSuper(Class<ASObject>::getRef());

	c->setVariableByQName("CASEINSENSITIVE","",abstract_d(CASEINSENSITIVE),DECLARED_TRAIT);
	c->setVariableByQName("DESCENDING","",abstract_d(DESCENDING),DECLARED_TRAIT);
	c->setVariableByQName("NUMERIC","",abstract_d(NUMERIC),DECLARED_TRAIT);
	c->setVariableByQName("RETURNINDEXEDARRAY","",abstract_d(RETURNINDEXEDARRAY),DECLARED_TRAIT);
	c->setVariableByQName("UNIQUESORT","",abstract_d(UNIQUESORT),DECLARED_TRAIT);

	// properties
	c->setDeclaredMethodByQName("length","",Class<IFunction>::getFunction(_getLength),GETTER_METHOD,true);
	c->setDeclaredMethodByQName("length","",Class<IFunction>::getFunction(_setLength),SETTER_METHOD,true);

	// public functions
	c->setDeclaredMethodByQName("concat",AS3,Class<IFunction>::getFunction(_concat),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("every",AS3,Class<IFunction>::getFunction(every),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("filter",AS3,Class<IFunction>::getFunction(filter),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("forEach",AS3,Class<IFunction>::getFunction(forEach),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("indexOf",AS3,Class<IFunction>::getFunction(indexOf),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("lastIndexOf",AS3,Class<IFunction>::getFunction(lastIndexOf),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("join",AS3,Class<IFunction>::getFunction(join),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("map",AS3,Class<IFunction>::getFunction(_map),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("pop",AS3,Class<IFunction>::getFunction(_pop),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("push",AS3,Class<IFunction>::getFunction(_push),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("reverse",AS3,Class<IFunction>::getFunction(_reverse),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("shift",AS3,Class<IFunction>::getFunction(shift),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("slice",AS3,Class<IFunction>::getFunction(slice),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("some",AS3,Class<IFunction>::getFunction(some),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("sort",AS3,Class<IFunction>::getFunction(_sort),NORMAL_METHOD,true);
	//c->setDeclaredMethodByQName("sortOn",AS3,Class<IFunction>::getFunction(sortOn),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("splice",AS3,Class<IFunction>::getFunction(splice),NORMAL_METHOD,true);
	c->setDeclaredMethodByQName("toLocaleString",AS3,Class<IFunction>::getFunction(_toString),NORMAL_METHOD,true);
	c->prototype->setVariableByQName("toString","",Class<IFunction>::getFunction(_toString),DYNAMIC_TRAIT);
	c->setDeclaredMethodByQName("unshift",AS3,Class<IFunction>::getFunction(unshift),NORMAL_METHOD,true);

	// workaround, pop was encountered not in the AS3 namespace before, need to investigate it further
	c->setDeclaredMethodByQName("pop","",Class<IFunction>::getFunction(_pop),NORMAL_METHOD,true);
}

void Array::buildTraits(ASObject* o)
{
}

ASFUNCTIONBODY(Array,_constructor)
{
	Array* th=static_cast<Array*>(obj);

	if(argslen==1 && (args[0]->getObjectType()==T_INTEGER || args[0]->getObjectType()==T_UINTEGER || args[0]->getObjectType()==T_NUMBER))
	{
		number_t size=args[0]->toNumber();
		if (size < 0 || size > UINT32_MAX)
			throw Class<RangeError>::getInstanceS("");
		LOG(LOG_CALLS,_("Creating array of length ") << size);
		th->resize((uint32_t)size);
	}
	else
	{
		LOG(LOG_CALLS,_("Called Array constructor"));
		th->resize(argslen);
		for(unsigned int i=0;i<argslen;i++)
		{
			th->set(i,args[i]);
			args[i]->incRef();
		}
	}
	return NULL;
}

ASFUNCTIONBODY(Array,generator)
{
	Array* th=Class<Array>::getInstanceS();
	if(argslen==1 && (args[0]->getObjectType()==T_INTEGER || args[0]->getObjectType()==T_UINTEGER || args[0]->getObjectType()==T_NUMBER))
	{
		number_t size=args[0]->toNumber();
		if (size < 0 || size > UINT32_MAX)
			throw Class<RangeError>::getInstanceS("");
		LOG(LOG_CALLS,_("Creating array of length ") << size);
		th->resize((uint32_t)size);
	}
	else
	{
		LOG(LOG_CALLS,_("Called Array constructor"));
		th->resize(argslen);
		for(unsigned int i=0;i<argslen;i++)
		{
			th->set(i,args[i]);
			args[i]->incRef();
		}
	}
	return th;
}

ASFUNCTIONBODY(Array,_concat)
{
	Array* th=static_cast<Array*>(obj);
	Array* ret=Class<Array>::getInstanceS();
	
	// copy values into new array
	ret->resize(th->size());
	std::map<uint32_t, data_slot>::iterator it=th->data.begin();
	for(;it != th->data.end();++it)
	{
		ret->data[it->first]=it->second;
	}
	
	if(argslen==1 && args[0]->getObjectType()==T_ARRAY)
	{
		Array* tmp=Class<Array>::cast(args[0]);
		std::map<uint32_t, data_slot>::iterator ittmp=tmp->data.begin();
		for(;ittmp != tmp->data.end();++ittmp)
		{
			ret->data[ret->size()+ittmp->first]=ittmp->second;
		}
		ret->resize(th->size()+tmp->size());
	}
	else
	{
		//Insert the arguments in the array
		for(unsigned int i=0;i<argslen;i++)
			ret->push(args[i]);
	}

	//All the elements in the new array should be increffed, as args will be deleted and
	//this array could die too
	for(unsigned int i=0;i<ret->size();i++)
	{
		if(ret->data.count(i) && ret->data[i].type==DATA_OBJECT && ret->data[i].data)
			ret->data[i].data->incRef();
	}

	return ret;
}

ASFUNCTIONBODY(Array,filter)
{
	Array* th=static_cast<Array*>(obj);
	assert_and_throw(argslen==1 || argslen==2);
	IFunction* f = static_cast<IFunction*>(args[0]);
	ASObject* params[3];
	Array* ret=Class<Array>::getInstanceS();
	ASObject *funcRet;

	for(unsigned int i=0;i<th->size();i++)
	{
		if (!th->data.count(i))
			continue;
		assert_and_throw(th->data[i].type==DATA_OBJECT);
		params[0] = th->data[i].data;
		th->data[i].data->incRef();
		params[1] = abstract_i(i);
		params[2] = th;
		th->incRef();

		if(argslen==1)
		{
			funcRet=f->call(new Null, params, 3);
		}
		else
		{
			args[1]->incRef();
			funcRet=f->call(args[1], params, 3);
		}
		if(funcRet)
		{
			if(Boolean_concrete(funcRet))
			{
				th->data[i].data->incRef();
				ret->push(th->data[i].data);
			}
			funcRet->decRef();
		}
	}
	return ret;
}

ASFUNCTIONBODY(Array, some)
{
	Array* th=static_cast<Array*>(obj);
	assert_and_throw(argslen==1 || argslen==2);
	IFunction* f = static_cast<IFunction*>(args[0]);
	ASObject* params[3];
	ASObject *funcRet;

	for(unsigned int i=0; i < th->size(); i++)
	{
		if (!th->data.count(i))
			continue;
		assert_and_throw(th->data[i].type==DATA_OBJECT);
		params[0] = th->data[i].data;
		th->data[i].data->incRef();
		params[1] = abstract_i(i);
		params[2] = th;
		th->incRef();

		if(argslen==1)
		{
			funcRet=f->call(new Null, params, 3);
		}
		else
		{
			args[1]->incRef();
			funcRet=f->call(args[1], params, 3);
		}
		if(funcRet)
		{
			if(Boolean_concrete(funcRet))
			{
				return funcRet;
			}
			funcRet->decRef();
		}
	}
	return abstract_b(false);
}

ASFUNCTIONBODY(Array, every)
{
	Array* th=static_cast<Array*>(obj);
	assert_and_throw(argslen==1 || argslen==2);
	IFunction* f = static_cast<IFunction*>(args[0]);
	ASObject* params[3];
	ASObject *funcRet;

	for(unsigned int i=0; i < th->size(); i++)
	{
		if (!th->data.count(i))
			continue;
		assert_and_throw(th->data[i].type==DATA_OBJECT);
		params[0] = th->data[i].data;
		th->data[i].data->incRef();
		params[1] = abstract_i(i);
		params[2] = th;
		th->incRef();

		if(argslen==1)
		{
			funcRet=f->call(new Null, params, 3);
		}
		else
		{
			args[1]->incRef();
			funcRet=f->call(args[1], params, 3);
		}
		if(funcRet)
		{
			if(!Boolean_concrete(funcRet))
			{
				return funcRet;
			}
			funcRet->decRef();
		}
	}
	return abstract_b(true);
}

ASFUNCTIONBODY(Array,_getLength)
{
	Array* th=static_cast<Array*>(obj);
	return abstract_ui(th->size());
}

ASFUNCTIONBODY(Array,_setLength)
{
	assert_and_throw(argslen == 1);
	Array* th=static_cast<Array*>(obj);
	uint32_t newLen=args[0]->toUInt();
	//If newLen is equal to size do nothing
	if(newLen==th->size())
		return NULL;
	th->resize(newLen);
	return NULL;
}

ASFUNCTIONBODY(Array,forEach)
{
	assert_and_throw(argslen == 1 || argslen == 2);
	Array* th=static_cast<Array*>(obj);
	IFunction* f = static_cast<IFunction*>(args[0]);
	ASObject* params[3];

	for(unsigned int i=0; i < th->size(); i++)
	{
		if (!th->data.count(i))
			continue;
		assert_and_throw(th->data[i].type==DATA_OBJECT);
		params[0] = th->data[i].data;
		th->data[i].data->incRef();
		params[1] = abstract_i(i);
		params[2] = th;
		th->incRef();

		ASObject *funcret;
		if( argslen == 1 )
		{
			funcret=f->call(new Null, params, 3);
		}
		else
		{
			args[1]->incRef();
			funcret=f->call(args[1], params, 3);
		}
		if(funcret)
			funcret->decRef();
	}

	return NULL;
}

ASFUNCTIONBODY(Array, _reverse)
{
	Array* th = static_cast<Array*>(obj);

	std::map<uint32_t, data_slot> tmp = std::map<uint32_t, data_slot>(th->data);
	uint32_t size = th->size();
	th->data.clear();
	std::map<uint32_t, data_slot>::iterator it=tmp.begin();
	for(;it != tmp.end();++it)
 	{
		th->data[size-(it->first+1)]=it->second;
	}
	th->incRef();
	return th;
}

ASFUNCTIONBODY(Array,lastIndexOf)
{
	Array* th=static_cast<Array*>(obj);
	assert_and_throw(argslen==1 || argslen==2);
	int ret=-1;
	ASObject* arg0=args[0];

	if(th->data.empty())
		return abstract_d(0);

	size_t i = th->size()-1;

	if(argslen == 2 && std::isnan(args[1]->toNumber()))
		return abstract_i(0);

	if(argslen == 2 && args[1]->getObjectType() != T_UNDEFINED && !std::isnan(args[1]->toNumber()))
	{
		int j = args[1]->toInt(); //Preserve sign
		if(j < 0) //Negative offset, use it as offset from the end of the array
		{
			if((size_t)-j > th->size())
				i = 0;
			else
				i = th->size()+j;
		}
		else //Positive offset, use it directly
		{
			if((size_t)j > th->size()) //If the passed offset is bigger than the array, cap the offset
				i = th->size()-1;
			else
				i = j;
		}
	}
	do
	{
		if (!th->data.count(i))
		    continue;
		DATA_TYPE dtype = th->data[i].type;
		assert_and_throw(dtype==DATA_OBJECT || dtype==DATA_INT);
		if((dtype == DATA_OBJECT && ABCVm::strictEqualImpl(th->data[i].data,arg0)) ||
			(dtype == DATA_INT && arg0->toInt() == th->data[i].data_i))
		{
			ret=i;
			break;
		}
	}
	while(i--);

	return abstract_i(ret);
}

ASFUNCTIONBODY(Array,shift)
{
	Array* th=static_cast<Array*>(obj);
	if(!th->size())
		return new Undefined;
	ASObject* ret;
	if(!th->data.count(0))
		ret = new Undefined;
	else
	{
		if(th->data[0].type==DATA_OBJECT)
			ret=th->data[0].data;
		else
			ret = abstract_i(th->data[0].data_i);
		th->data.erase(0);
	}
	for(uint32_t i= 1;i< th->size();i++)
	{
		if (th->data.count(i))
		{
			th->data[i-1]=th->data[i];
		}
		else if (th->data.count(i-1))
		{
			th->data.erase(i-1);
		}
	}
	th->data.erase(th->size()-1);// erase here to avoid decref
	th->resize(th->size()-1);
	return ret;
}

int Array::capIndex(int i) const
{
	int totalSize=size();

	if(totalSize <= 0)
		return 0;
	else if(i < -totalSize)
		return 0;
	else if(i > totalSize)
		return totalSize;
	else if(i>=0)     // 0 <= i < totalSize
		return i;
	else              // -totalSize <= i < 0
	{
		//A negative i is relative to the end
		return i+totalSize;
	}
}

ASFUNCTIONBODY(Array,slice)
{
	Array* th=static_cast<Array*>(obj);

	int startIndex=0;
	int endIndex=16777215;
	if(argslen>0)
		startIndex=args[0]->toInt();
	if(argslen>1)
		endIndex=args[1]->toInt();

	startIndex=th->capIndex(startIndex);
	endIndex=th->capIndex(endIndex);

	Array* ret=Class<Array>::getInstanceS();
	int j = 0;
	for(int i=startIndex; i<endIndex; i++) 
	{
		if (th->data.count(i))
		{
			if(th->data[i].type == DATA_OBJECT)
				th->data[i].data->incRef();
			ret->data[j]=th->data[i];
		}
		j++;
	}
	ret->resize(j);
	return ret;
}

ASFUNCTIONBODY(Array,splice)
{
	Array* th=static_cast<Array*>(obj);
	assert_and_throw(argslen >= 1);
	int startIndex=args[0]->toInt();
	//By default, delete all the element up to the end
	//Use the array len, it will be capped below
	int deleteCount=th->size();
	if(argslen > 1)
		deleteCount=args[1]->toUInt();

	int totalSize=th->size();
	Array* ret=Class<Array>::getInstanceS();

	startIndex=th->capIndex(startIndex);

	if((startIndex+deleteCount)>totalSize)
		deleteCount=totalSize-startIndex;

	ret->resize(deleteCount);
	if(deleteCount)
	{
		// write deleted items to return array
		for(int i=0;i<deleteCount;i++)
		{
			if (th->data.count(startIndex+i))
				ret->data[i] = th->data[startIndex+i];
		}
		// delete items from current array
		for (int i = 0; i < deleteCount; i++)
		{
			if(th->data.count(startIndex+i))
			{
				th->data.erase(startIndex+i);
			}
		}
	}
	// remember items in current array that have to be moved to new position
	vector<data_slot> tmp = vector<data_slot>(totalSize- (startIndex+deleteCount));
	for (int i = startIndex+deleteCount; i < totalSize ; i++)
	{
		if (th->data.count(i))
		{
			tmp[i-(startIndex+deleteCount)] = th->data[i];
			th->data.erase(i);
		}
	}
	th->resize(startIndex);

	
	//Insert requested values starting at startIndex
	for(unsigned int i=2;i<argslen;i++)
	{
		args[i]->incRef();
		th->push(args[i]);
	}
	// move remembered items to new position
	for(int i=0;i<totalSize- (startIndex+deleteCount);i++)
	{
		if (tmp[i].type != DATA_OBJECT || tmp[i].data != NULL)
			th->data[startIndex+i+(argslen > 2 ? argslen-2 : 0)] = tmp[i];
	}
	th->resize((totalSize-deleteCount)+(argslen > 2 ? argslen-2 : 0));
	return ret;
}

ASFUNCTIONBODY(Array,join)
{
	Array* th=static_cast<Array*>(obj);
	
	tiny_string del = ",";
	if (argslen == 1)
	      del=args[0]->toString();
	string ret;
	for(uint32_t i=0;i<th->size();i++)
	{
		ret+=th->at(i)->toString().raw_buf();
		if(i!=th->size()-1)
			ret+=del.raw_buf();
	}
	return Class<ASString>::getInstanceS(ret);
}

ASFUNCTIONBODY(Array,indexOf)
{
	Array* th=static_cast<Array*>(obj);
	assert_and_throw(argslen==1 || argslen==2);
	int ret=-1;
	ASObject* arg0=args[0];

	int unsigned i = 0;
	if(argslen == 2)
	{
		i = args[1]->toInt();
	}

	DATA_TYPE dtype;
	for(;i<th->size();i++)
	{
		if (!th->data.count(i))
			continue;
		dtype = th->data[i].type;
		assert_and_throw(dtype==DATA_OBJECT || dtype==DATA_INT);
		if((dtype == DATA_OBJECT && ABCVm::strictEqualImpl(th->data[i].data,arg0)) ||
			(dtype == DATA_INT && arg0->toInt() == th->data[i].data_i))
		{
			ret=i;
			break;
		}
	}
	return abstract_i(ret);
}


ASFUNCTIONBODY(Array,_pop)
{
	Array* th=static_cast<Array*>(obj);
	uint32_t size =th->size();
	if (size == 0)
		return new Undefined;
	ASObject* ret;
	if (th->data.count(size-1))
	{
		if(th->data[size-1].type==DATA_OBJECT)
			ret=th->data[size-1].data;
		else
			ret = abstract_i(th->data[size-1].data_i);
		th->data.erase(size-1);
	}
	else
		ret = new Undefined;

	th->currentsize--;
	return ret;
}

bool Array::sortComparatorDefault::operator()(const data_slot& d1, const data_slot& d2)
{
	if(isNumeric)
	{
		number_t a=numeric_limits<double>::quiet_NaN();
		number_t b=numeric_limits<double>::quiet_NaN();
		if(d1.type==DATA_INT)
			a=d1.data_i;
		else if(d1.type==DATA_OBJECT && d1.data)
			a=d1.data->toNumber();

		if(d2.type==DATA_INT)
			b=d2.data_i;
		else if(d2.type==DATA_OBJECT && d2.data)
			b=d2.data->toNumber();

		if(std::isnan(a) || std::isnan(b))
			throw RunTimeException("Cannot sort non number with Array.NUMERIC option");
		return a<b;
	}
	else
	{
		//Comparison is always in lexicographic order
		tiny_string s1;
		tiny_string s2;
		if(d1.type==DATA_INT)
			s1=Integer::toString(d1.data_i);
		else if(d1.type==DATA_OBJECT && d1.data)
			s1=d1.data->toString();
		else
			s1="undefined";
		if(d2.type==DATA_INT)
			s2=Integer::toString(d2.data_i);
		else if(d2.type==DATA_OBJECT && d2.data)
			s2=d2.data->toString();
		else
			s2="undefined";

		//TODO: unicode support
		if(isCaseInsensitive)
			return s1.strcasecmp(s2)<0;
		else
			return s1<s2;
	}
}

bool Array::sortComparatorWrapper::operator()(const data_slot& d1, const data_slot& d2)
{
	ASObject* objs[2];
	if(d1.type==DATA_INT)
		objs[0]=abstract_i(d1.data_i);
	else if(d1.type==DATA_OBJECT && d1.data)
	{
		objs[0]=d1.data;
		objs[0]->incRef();
	}
	else
		objs[0]=new Undefined;

	if(d2.type==DATA_INT)
		objs[1]=abstract_i(d2.data_i);
	else if(d2.type==DATA_OBJECT && d2.data)
	{
		objs[1]=d2.data;
		objs[1]->incRef();
	}
	else
		objs[1]=new Undefined;

	assert(comparator);
	_NR<ASObject> ret=_MNR(comparator->call(new Null, objs, 2));
	assert_and_throw(ret);
	return (ret->toInt()<0); //Less
}

ASFUNCTIONBODY(Array,_sort)
{
	Array* th=static_cast<Array*>(obj);
	IFunction* comp=NULL;
	bool isNumeric=false;
	bool isCaseInsensitive=false;
	for(uint32_t i=0;i<argslen;i++)
	{
		if(args[i]->getObjectType()==T_FUNCTION) //Comparison func
		{
			assert_and_throw(comp==NULL);
			comp=static_cast<IFunction*>(args[i]);
		}
		else
		{
			uint32_t options=args[i]->toInt();
			if(options&NUMERIC)
				isNumeric=true;
			if(options&CASEINSENSITIVE)
				isCaseInsensitive=true;
			if(options&(~(NUMERIC|CASEINSENSITIVE)))
				throw UnsupportedException("Array::sort not completely implemented");
		}
	}
	std::vector<data_slot> tmp = vector<data_slot>(th->data.size());
	std::map<uint32_t, data_slot>::iterator it=th->data.begin();
	int i = 0;
	for(;it != th->data.end();++it)
	{
		tmp[i++]= it->second;
	}
	
	if(comp)
		sort(tmp.begin(),tmp.end(),sortComparatorWrapper(comp));
	else
		sort(tmp.begin(),tmp.end(),sortComparatorDefault(isNumeric,isCaseInsensitive));

	th->data.clear();
	std::vector<data_slot>::iterator ittmp=tmp.begin();
	i = 0;
	for(;ittmp != tmp.end();++ittmp)
	{
		th->data[i++]= *ittmp;
	}
	obj->incRef();
	return obj;
}

ASFUNCTIONBODY(Array,sortOn)
{
//	Array* th=static_cast<Array*>(obj);
/*	if(th->data.size()>1)
		throw UnsupportedException("Array::sort not completely implemented");
	LOG(LOG_NOT_IMPLEMENTED,_("Array::sort not really implemented"));*/
	obj->incRef();
	return obj;
}

ASFUNCTIONBODY(Array,unshift)
{
	Array* th=static_cast<Array*>(obj);
	th->resize(th->size()+argslen);
	for(uint32_t i=th->size();i> 0;i--)
	{
		if (th->data.count(i-1))
		{
			th->data[(i-1)+argslen]=th->data[i-1];
			th->data.erase(i-1);
		}
		
	}

	for(uint32_t i=0;i<argslen;i++)
	{
		th->data[i] = data_slot(args[i],DATA_OBJECT);
		args[i]->incRef();
	}
	return abstract_i(th->size());
}

ASFUNCTIONBODY(Array,_push)
{
	Array* th=static_cast<Array*>(obj);
	for(unsigned int i=0;i<argslen;i++)
	{
		if (th->size() >= UINT32_MAX)
			throw Class<RangeError>::getInstanceS("");
			
		th->push(args[i]);
		args[i]->incRef();
	}
	return abstract_i(th->size());
}

ASFUNCTIONBODY(Array,_map)
{
	Array* th=static_cast<Array*>(obj);
	assert_and_throw(argslen==1 && args[0]->getObjectType()==T_FUNCTION);
	IFunction* func=static_cast<IFunction*>(args[0]);
	Array* arrayRet=Class<Array>::getInstanceS();

	for(uint32_t i=0;i<th->size();i++)
	{
		ASObject* funcArgs[3];
		if (!th->data.count(i))
			funcArgs[0]=new Null;
		else
		{
			const data_slot& slot=th->data[i];
			if(slot.type==DATA_INT)
				funcArgs[0]=abstract_i(slot.data_i);
			else if(slot.type==DATA_OBJECT && slot.data)
			{
				funcArgs[0]=slot.data;
				funcArgs[0]->incRef();
			}
			else
				funcArgs[0]=new Undefined;
		}
		funcArgs[1]=abstract_i(i);
		funcArgs[2]=th;
		funcArgs[2]->incRef();
		ASObject* funcRet=func->call(new Null, funcArgs, 3);
		assert_and_throw(funcRet);
		arrayRet->push(funcRet);
	}

	return arrayRet;
}

ASFUNCTIONBODY(Array,_toString)
{
	Array* th=static_cast<Array*>(obj);
	return Class<ASString>::getInstanceS(th->toString_priv());
}

int32_t Array::getVariableByMultiname_i(const multiname& name)
{
	assert_and_throw(implEnable);
	unsigned int index=0;
	if(!isValidMultiname(name,index))
		return ASObject::getVariableByMultiname_i(name);

	if(index<size())
	{
		if (!data.count(index))
			return 0;
		switch(data[index].type)
		{
			case DATA_OBJECT:
			{
				assert(data[index].data!=NULL);
				if(data[index].data->getObjectType()==T_INTEGER)
				{
					Integer* i=static_cast<Integer*>(data[index].data);
					return i->toInt();
				}
				else if(data[index].data->getObjectType()==T_NUMBER)
				{
					Number* i=static_cast<Number*>(data[index].data);
					return i->toInt();
				}
				else
					throw UnsupportedException("Array::getVariableByMultiname_i not completely implemented");
			}
			case DATA_INT:
				return data[index].data_i;
		}
	}

	return ASObject::getVariableByMultiname_i(name);
}

_NR<ASObject> Array::getVariableByMultiname(const multiname& name, GET_VARIABLE_OPTION opt)
{
	if((opt & SKIP_IMPL)!=0 || !implEnable)
		return ASObject::getVariableByMultiname(name,opt);

	assert_and_throw(name.ns.size()>0);
	if(name.ns[0].name!="")
		return ASObject::getVariableByMultiname(name,opt);

	unsigned int index=0;
	if(!isValidMultiname(name,index))
		return ASObject::getVariableByMultiname(name,opt);
	if(index<size())
	{
		ASObject* ret=NULL;
		if (!data.count(index))
			ret = new Undefined;
		switch(data[index].type)
		{
			case DATA_OBJECT:
				ret=data[index].data;
				if(ret==NULL)
				{
					ret=new Undefined;
					data[index].data=ret;
				}
				ret->incRef();
				break;
			case DATA_INT:
				ret=abstract_i(data[index].data_i);
				break;
		}
		return _MNR(ret);
	}
	else
		return NullRef;
}

void Array::setVariableByMultiname_i(const multiname& name, int32_t value)
{
	assert_and_throw(implEnable);
	unsigned int index=0;
	if(!isValidMultiname(name,index))
	{
		ASObject::setVariableByMultiname_i(name,value);
		return;
	}
	if(index>=size())
		resize(index+1);

	if(data.count(index) && data[index].type==DATA_OBJECT && data[index].data)
		data[index].data->decRef();
	if(!data.count(index))
		data[index] = data_slot();
	data[index].data_i=value;
	data[index].type=DATA_INT;
}


bool Array::hasPropertyByMultiname(const multiname& name, bool considerDynamic)
{
	if(considerDynamic==false)
		return ASObject::hasPropertyByMultiname(name, considerDynamic);

	uint32_t index=0;
	if(!isValidMultiname(name,index))
		return ASObject::hasPropertyByMultiname(name, considerDynamic);

	return (index<size()) && (data.count(index));
}

bool Array::isValidMultiname(const multiname& name, uint32_t& index)
{
	//First of all the multiname has to contain the null namespace
	//As the namespace vector is sorted, we check only the first one
	assert_and_throw(name.ns.size()!=0);
	if(name.ns[0].name!="")
		return false;

	index=0;
	switch(name.name_type)
	{
		//We try to convert this to an index, otherwise bail out
		case multiname::NAME_STRING:
			if(name.name_s.empty())
				return false;
			for(auto i=name.name_s.begin(); i!=name.name_s.end(); ++i)
			{
				if(!i.isdigit())
					return false;

				index*=10;
				index+=i.digit_value();
			}
			break;
		//This is already an int, so its good enough
		case multiname::NAME_INT:
			if(name.name_i < 0)
				return false;
			index=name.name_i;
			break;
		case multiname::NAME_NUMBER:
			if(!Number::isInteger(name.name_d))
				return false;
			index = name.name_d;
			break;
		case multiname::NAME_OBJECT:
			//TODO: should be use toPrimitive here?
			return false;
		default:
			throw UnsupportedException("Array::isValidMultiname not completely implemented");
	}
	return true;
}

void Array::setVariableByMultiname(const multiname& name, ASObject* o)
{
	assert_and_throw(implEnable);
	uint32_t index=0;
	if(!isValidMultiname(name,index))
		return ASObject::setVariableByMultiname(name,o);

	if(index>=size())
		resize((uint64_t)index+1);

	if(data.count(index) && data[index].type==DATA_OBJECT && data[index].data)
		data[index].data->decRef();
	if(!data.count(index))
		data[index] = data_slot();

	if(o->getObjectType()==T_INTEGER)
	{
		Integer* i=static_cast<Integer*>(o);
		data[index].data_i=i->val;
		data[index].type=DATA_INT;
		o->decRef();
	}
	else
	{
		data[index].data=o;
		data[index].type=DATA_OBJECT;
	}
}

bool Array::deleteVariableByMultiname(const multiname& name)
{
	assert_and_throw(implEnable);
	unsigned int index=0;
	if(!isValidMultiname(name,index))
		return ASObject::deleteVariableByMultiname(name);

	if(index>=size())
		return true;
	if (!data.count(index))
		return true;
	if(data[index].type==DATA_OBJECT && data[index].data)
		data[index].data->decRef();

	data.erase(index);
	return true;
}

bool Array::isValidQName(const tiny_string& name, const tiny_string& ns, unsigned int& index)
{
	if(ns!="")
		return false;
	assert_and_throw(!name.empty());
	index=0;
	//First we try to convert the string name to an index, at the first non-digit
	//we bail out
	for(auto i=name.begin(); i!=name.end(); ++i)
	{
		if(!i.isdigit())
			return false;

		index*=10;
		index+=i.digit_value();
	}
	return true;
}

tiny_string Array::toString()
{
	assert_and_throw(implEnable);
	return toString_priv();
}

tiny_string Array::toString_priv() const
{
	string ret;
	for(uint32_t i=0;i<size();i++)
	{
		if (data.count(i))
		{
			if(data.at(i).type==DATA_OBJECT)
			{
				if(data.at(i).data)
					ret+=data.at(i).data->toString().raw_buf();
			}
			else if(data.at(i).type==DATA_INT)
			{
				char buf[20];
				snprintf(buf,20,"%i",data.at(i).data_i);
				ret+=buf;
			}
			else
				throw UnsupportedException("Array::toString not completely implemented");
		}
		if(i!=size()-1)
			ret+=',';
	}
	return ret;
}

_R<ASObject> Array::nextValue(uint32_t index)
{
	assert_and_throw(implEnable);
	if(index<=size())
	{
		index--;
		if(!data.count(index))
			return _MR(new Undefined);
		if(data[index].type==DATA_OBJECT)
		{
			if(data[index].data==NULL)
				return _MR(new Undefined);
			else
			{
				data[index].data->incRef();
				return _MR(data[index].data);
			}
		}
		else if(data[index].type==DATA_INT)
			return _MR(abstract_i(data[index].data_i));
		else
			throw UnsupportedException("Unexpected data type");
	}
	else
	{
		//Fall back on object properties
		return ASObject::nextValue(index-size());
	}
}

uint32_t Array::nextNameIndex(uint32_t cur_index)
{
	assert_and_throw(implEnable);
	if(cur_index<size())
	{
		while (!data.count(cur_index) && cur_index<size())
		{
			cur_index++;
		}
		if(cur_index<size())
			return cur_index+1;
		else
			return 0;
	}
	else
	{
		//Fall back on object properties
		uint32_t ret=ASObject::nextNameIndex(cur_index-size());
		if(ret==0)
			return 0;
		else
			return ret+size();

	}
}

_R<ASObject> Array::nextName(uint32_t index)
{
	assert_and_throw(implEnable);
	if(index<=size())
		return _MR(abstract_i(index-1));
	else
	{
		//Fall back on object properties
		return ASObject::nextName(index-size());
	}
}

ASObject* Array::at(unsigned int index) const
{
	if(size()<=index)
		outofbounds();

	if (!data.count(index))
		return new Undefined;
	switch(data.at(index).type)
	{
		case DATA_OBJECT:
		{
			if(data.at(index).data)
				return data.at(index).data;
		}
		case DATA_INT:
			return abstract_i(data.at(index).data_i);
	}

	//We should be here only if data is an object and is NULL
	return new Undefined;
}

void Array::outofbounds() const
{
	throw ParseException("Array access out of bounds");
}

void Array::serialize(ByteArray* out, std::map<tiny_string, uint32_t>& stringMap,
				std::map<const ASObject*, uint32_t>& objMap,
				std::map<const Class_base*, uint32_t>& traitsMap)
{
	assert_and_throw(objMap.find(this)==objMap.end());
	out->writeByte(array_marker);
	//Check if the array has been already serialized
	auto it=objMap.find(this);
	if(it!=objMap.end())
	{
		//The least significant bit is 0 to signal a reference
		out->writeU29(it->second << 1);
	}
	else
	{
		//Add the array to the map
		objMap.insert(make_pair(this, objMap.size()));

		uint32_t denseCount = size();
		assert_and_throw(denseCount<0x20000000);
		uint32_t value = (denseCount << 1) | 1;
		out->writeU29(value);
		serializeDynamicProperties(out, stringMap, objMap, traitsMap);
		for(uint32_t i=0;i<denseCount;i++)
		{
			if (!data.count(i))
				throw UnsupportedException("undefined not supported in Array::serialize");
			switch(data.at(i).type)
			{
				case DATA_INT:
					throw UnsupportedException("int not supported in Array::serialize");
				case DATA_OBJECT:
					data.at(i).data->serialize(out, stringMap, objMap, traitsMap);
			}
		}
	}
}

void Array::finalize()
{
	ASObject::finalize();
	for(unsigned int i=0;i<size();i++)
	{
		if(data.count(i) && data[i].type==DATA_OBJECT && data[i].data)
			data[i].data->decRef();
	}
	data.clear();
}


