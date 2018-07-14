/*
*******************************************************************************
\file obj.h
\brief Compound objects
\project bee2 [cryptographic library]
\author (C) Sergey Agievich [agievich@{bsu.by|gmail.com}]
\created 2014.04.14
\version 2014.04.17
\license This program is released under the GNU General Public License 
version 3. See Copyright Notices in bee2/info.h.
*******************************************************************************
*/

/*!
*******************************************************************************
\file obj.h
\brief Составные объекты
*******************************************************************************
*/

#ifndef __BEE2_OBJ_H
#define __BEE2_OBJ_H

#include "bee2/defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
*******************************************************************************
\file obj.h

Объект представляет собой размеченный фрагмент памяти. Для разметки 
используются указатели, которые ссылаются на различные участки фрагмента. 
Ссылочными участками могут быть другие объекты, которые называются 
вложенными. Ссылки на внутренние участки обновляются при перемещении
объекта.

Объект начинается с заголовка типа obj_hdr_t. В заголовке указывается размер 
участка памяти, занимаемой объектом (keep), количество указателей (p_count) 
и количество указателей на ссылочные объекты (o_count). 

После заголовка следует таблица указателей. Указатели на объекты помещаются 
в начало списка.

Указатели в таблице могут ссылаться не только на внутренние участки памяти, 
но и на внешние, т.е. лежащие вне фрагмента, принадлежащего объекту. 
Внешние ссылки остаются постоянными при перемещении объекта. 
******************************************************************************
*/

/*! \brief Заголовок объекта */
typedef struct
{
	size_t keep;		/*!< размер объекта */
	size_t p_count;		/*!< число указателей */
	size_t o_count;		/*!< число ссылочных объектов */
} obj_hdr_t;

/*! \brief Размер объекта */
#define objKeep(obj)\
	(((const obj_hdr_t*)(obj))->keep)

/*! \brief Число указателей */
#define objPCount(obj)\
	(((const obj_hdr_t*)(obj))->p_count)

/*! \brief Число указателей на объекты */
#define objOCount(obj)\
	(((const obj_hdr_t*)(obj))->o_count)

/*! \brief Указатель 
	
	Определяется i-й элемент таблицы указателей, который интерпретируется 
	как указатель на тип type.
*/
#define objPtr(obj, i, type)\
	(((type**)((octet*)(obj) + sizeof(obj_hdr_t)))[i])

/*! \brief Const-указатель 
	
	Определяется i-й элемент таблицы указателей, который интерпретируется 
	как указатель на тип const type.
*/
#define objCPtr(obj, i, type)\
	(((const type* const*)((const octet*)(obj) + sizeof(obj_hdr_t)))[i])

/*! \brief Окончание объекта 

	Определяется адрес окончания фрагмента памяти, занимаемой объектом obj.
	Адрес интерпретируется как указатель на тип type
*/
#define objEnd(obj, type)\
	((type*)((octet*)(obj) + objKeep(obj)))

/*
*******************************************************************************
Управление объектом
*******************************************************************************
*/

/*!	\brief Работоспособный объект?

	Проверяется работоспособность объекта obj и вложенных в него объектов.
	\return Признак работоспособности.
*/
bool_t objIsOperable(
	const void* obj		/*!< [in] объект */
);

/*!	\brief Работоспособный отдельный объект?

	Проверяется работоспособность объекта obj. Работоспособность вложенных 
	в obj объектов не проверяется.
	\return Признак работоспособности.
*/
bool_t objIsOperable2(
	const void* obj		/*!< [in] объект */
);

/*!	\brief Копирование объекта

	Объект src копируется по адресу dest.
	\pre Объект src работоспособен.
	\pre По адресу dest зарезервировано objKeep(src) октетов.
	\remark При копировании настраиваются указатели, в том числе 
	указатели на вложенные объекты.
*/
void objCopy(
	void* dest,			/*!< [out] объект-назначение */
	const void* src		/*!< [in] объект-источник */
);

/*!	\brief Присоединение объекта

	Объект src записывается в конец объекта src. В i-ую ячейку таблицы
	указателей dest записывается ссылка на копию src. Длина dest увеличивается
	на длину src.
	\pre Объекты src и dest работоспособны.
	\pre По адресу objEnd(dest, void) зарезервировано objKeep(src) октетов.
	\pre i < objOCount(dest).
*/
void objAppend(
	void* dest,			/*!< [out] объект-контейнер */
	const void* src,	/*!< [in] присоединяемый объект */
	size_t i			/*!< [in] номер присоединяемого объекта */
);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BEE2_OBJ_H */
