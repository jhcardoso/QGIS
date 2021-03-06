/***************************************************************************
    qgsvectordataprovider.cpp - DataProvider Interface for vector layers
     --------------------------------------
    Date                 : 26-Oct-2004
    Copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSettings>
#include <QTextCodec>

#include <cfloat> // for DBL_MAX
#include <climits>
#include <limits>

#include "qgsvectordataprovider.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgsgeometryfactory.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

QgsVectorDataProvider::QgsVectorDataProvider( const QString& uri )
    : QgsDataProvider( uri )
    , mCacheMinMaxDirty( true )
{
  QSettings settings;
  setEncoding( settings.value( QStringLiteral( "/UI/encoding" ), "System" ).toString() );
}


QgsVectorDataProvider::~QgsVectorDataProvider()
{
}

QString QgsVectorDataProvider::storageType() const
{
  return QStringLiteral( "Generic vector file" );
}

QString QgsVectorDataProvider::dataComment() const
{
  return QString();
}

bool QgsVectorDataProvider::addFeatures( QgsFeatureList &flist )
{
  Q_UNUSED( flist );
  return false;
}

bool QgsVectorDataProvider::deleteFeatures( const QgsFeatureIds &ids )
{
  Q_UNUSED( ids );
  return false;
}

bool QgsVectorDataProvider::addAttributes( const QList<QgsField> &attributes )
{
  Q_UNUSED( attributes );
  return false;
}

bool QgsVectorDataProvider::deleteAttributes( const QgsAttributeIds &attributes )
{
  Q_UNUSED( attributes );
  return false;
}

bool QgsVectorDataProvider::renameAttributes( const QgsFieldNameMap& renamedAttributes )
{
  Q_UNUSED( renamedAttributes );
  return false;
}

bool QgsVectorDataProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  Q_UNUSED( attr_map );
  return false;
}

QVariant QgsVectorDataProvider::defaultValue( int fieldId ) const
{
  Q_UNUSED( fieldId );
  return QVariant();
}

QString QgsVectorDataProvider::defaultValueClause( int fieldIndex ) const
{
  Q_UNUSED( fieldIndex );
  return QString();
}

QgsFieldConstraints::Constraints QgsVectorDataProvider::fieldConstraints( int fieldIndex ) const
{
  QgsFields f = fields();
  if ( fieldIndex < 0 || fieldIndex >= f.count() )
    return 0;

  return f.at( fieldIndex ).constraints().constraints();
}

bool QgsVectorDataProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  Q_UNUSED( geometry_map );
  return false;
}

bool QgsVectorDataProvider::changeFeatures( const QgsChangedAttributesMap &attr_map,
    const QgsGeometryMap &geometry_map )
{
  Q_UNUSED( attr_map );
  Q_UNUSED( geometry_map );
  return false;
}

bool QgsVectorDataProvider::createSpatialIndex()
{
  return false;
}

bool QgsVectorDataProvider::createAttributeIndex( int field )
{
  Q_UNUSED( field );
  return true;
}

QgsVectorDataProvider::Capabilities QgsVectorDataProvider::capabilities() const
{
  return QgsVectorDataProvider::NoCapabilities;
}


void QgsVectorDataProvider::setEncoding( const QString& e )
{
  mEncoding = QTextCodec::codecForName( e.toLocal8Bit().constData() );

  if ( !mEncoding && e != QLatin1String( "System" ) )
  {
    QgsMessageLog::logMessage( tr( "Codec %1 not found. Falling back to system locale" ).arg( e ) );
    mEncoding = QTextCodec::codecForName( "System" );
  }

  if ( !mEncoding )
    mEncoding = QTextCodec::codecForLocale();

  Q_ASSERT( mEncoding );
}

QString QgsVectorDataProvider::encoding() const
{
  if ( mEncoding )
  {
    return mEncoding->name();
  }

  return QLatin1String( "" );
}

QString QgsVectorDataProvider::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

  if ( abilities & QgsVectorDataProvider::AddFeatures )
  {
    abilitiesList += tr( "Add Features" );
    QgsDebugMsg( "Capability: Add Features" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteFeatures )
  {
    abilitiesList += tr( "Delete Features" );
    QgsDebugMsg( "Capability: Delete Features" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeAttributeValues )
  {
    abilitiesList += tr( "Change Attribute Values" );
    QgsDebugMsg( "Capability: Change Attribute Values" );
  }

  if ( abilities & QgsVectorDataProvider::AddAttributes )
  {
    abilitiesList += tr( "Add Attributes" );
    QgsDebugMsg( "Capability: Add Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::DeleteAttributes )
  {
    abilitiesList += tr( "Delete Attributes" );
    QgsDebugMsg( "Capability: Delete Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::RenameAttributes )
  {
    abilitiesList += tr( "Rename Attributes" );
    QgsDebugMsg( "Capability: Rename Attributes" );
  }

  if ( abilities & QgsVectorDataProvider::CreateSpatialIndex )
  {
    // TODO: Tighten up this test.  See QgsOgrProvider for details.
    abilitiesList += tr( "Create Spatial Index" );
    QgsDebugMsg( "Capability: Create Spatial Index" );
  }

  if ( abilities & QgsVectorDataProvider::CreateAttributeIndex )
  {
    abilitiesList += tr( "Create Attribute Indexes" );
    QgsDebugMsg( "Capability: Create Attribute Index" );
  }

  if ( abilities & QgsVectorDataProvider::SelectAtId )
  {
    abilitiesList += tr( "Fast Access to Features at ID" );
    QgsDebugMsg( "Capability: Select at ID" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeGeometries )
  {
    abilitiesList += tr( "Change Geometries" );
    QgsDebugMsg( "Capability: Change Geometries" );
  }

  if ( abilities & QgsVectorDataProvider::SimplifyGeometries )
  {
    abilitiesList += tr( "Presimplify Geometries" );
    QgsDebugMsg( "Capability: Simplify Geometries before fetching the feature" );
  }

  if ( abilities & QgsVectorDataProvider::SimplifyGeometriesWithTopologicalValidation )
  {
    abilitiesList += tr( "Presimplify Geometries with Validity Check" );
    QgsDebugMsg( "Capability: Simplify Geometries before fetch the feature ensuring that the result is a valid geometry" );
  }

  if ( abilities & QgsVectorDataProvider::ChangeFeatures )
  {
    abilitiesList += tr( "Simultaneous Geometry and Attribute Updates" );
    QgsDebugMsg( "Capability: change both feature attributes and geometry at once" );
  }

  if ( abilities & QgsVectorDataProvider::TransactionSupport )
  {
    abilitiesList += tr( "Transactions" );
    QgsDebugMsg( "Capability: transactions" );
  }

  if ( abilities & QgsVectorDataProvider::CircularGeometries )
  {
    abilitiesList += tr( "Curved Geometries" );
    QgsDebugMsg( "Supports circular geometry types (circularstring, compoundcurve, curvepolygon)" );
  }

  return abilitiesList.join( QStringLiteral( ", " ) );
}


int QgsVectorDataProvider::fieldNameIndex( const QString& fieldName ) const
{
  return fields().lookupField( fieldName );
}

QMap<QString, int> QgsVectorDataProvider::fieldNameMap() const
{
  QMap<QString, int> resultMap;

  QgsFields theFields = fields();
  for ( int i = 0; i < theFields.count(); ++i )
  {
    resultMap.insert( theFields.at( i ).name(), i );
  }

  return resultMap;
}

QgsAttributeList QgsVectorDataProvider::attributeIndexes() const
{
  return fields().allAttributesList();
}

QgsAttributeList QgsVectorDataProvider::pkAttributeIndexes() const
{
  return QgsAttributeList();
}

const QList< QgsVectorDataProvider::NativeType > &QgsVectorDataProvider::nativeTypes() const
{
  return mNativeTypes;
}

QgsAttrPalIndexNameHash QgsVectorDataProvider::palAttributeIndexNames() const
{
  return QgsAttrPalIndexNameHash();
}

bool QgsVectorDataProvider::supportedType( const QgsField &field ) const
{
  QgsDebugMsgLevel( QString( "field name = %1 type = %2 length = %3 precision = %4" )
                    .arg( field.name(),
                          QVariant::typeToName( field.type() ) )
                    .arg( field.length() )
                    .arg( field.precision() ), 2 );

  Q_FOREACH ( const NativeType& nativeType, mNativeTypes )
  {
    QgsDebugMsgLevel( QString( "native field type = %1 min length = %2 max length = %3 min precision = %4 max precision = %5" )
                      .arg( QVariant::typeToName( nativeType.mType ) )
                      .arg( nativeType.mMinLen )
                      .arg( nativeType.mMaxLen )
                      .arg( nativeType.mMinPrec )
                      .arg( nativeType.mMaxPrec ), 2 );

    if ( field.type() != nativeType.mType )
      continue;

    if ( field.length() == -1 )
    {
      // source length unlimited
      if ( nativeType.mMinLen > -1 || nativeType.mMaxLen > -1 )
      {
        // destination limited
        continue;
      }
    }
    else
    {
      // source length limited
      if ( nativeType.mMinLen > -1 && nativeType.mMaxLen > -1 &&
           ( field.length() < nativeType.mMinLen || field.length() > nativeType.mMaxLen ) )
      {
        // source length exceeds destination limits
        continue;
      }
    }

    if ( field.precision() == -1 )
    {
      // source precision unlimited / n/a
      if ( nativeType.mMinPrec > -1 || nativeType.mMaxPrec > -1 )
      {
        // destination limited
        continue;
      }
    }
    else
    {
      // source precision unlimited / n/a
      if ( nativeType.mMinPrec > -1 && nativeType.mMaxPrec > -1 &&
           ( field.precision() < nativeType.mMinPrec || field.precision() > nativeType.mMaxPrec ) )
      {
        // source precision exceeds destination limits
        continue;
      }
    }

    QgsDebugMsg( "native type matches" );
    return true;
  }

  QgsDebugMsg( "no sufficient native type found" );
  return false;
}

QVariant QgsVectorDataProvider::minimumValue( int index ) const
{
  if ( index < 0 || index >= fields().count() )
  {
    QgsDebugMsg( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  fillMinMaxCache();

  if ( !mCacheMinValues.contains( index ) )
    return QVariant();

  return mCacheMinValues[index];
}

QVariant QgsVectorDataProvider::maximumValue( int index ) const
{
  if ( index < 0 || index >= fields().count() )
  {
    QgsDebugMsg( "Warning: access requested to invalid field index: " + QString::number( index ) );
    return QVariant();
  }

  fillMinMaxCache();

  if ( !mCacheMaxValues.contains( index ) )
    return QVariant();

  return mCacheMaxValues[index];
}

void QgsVectorDataProvider::uniqueValues( int index, QList<QVariant> &values, int limit ) const
{
  QgsFeature f;
  QgsAttributeList keys;
  keys.append( index );
  QgsFeatureIterator fi = getFeatures( QgsFeatureRequest().setSubsetOfAttributes( keys ).setFlags( QgsFeatureRequest::NoGeometry ) );

  QSet<QString> set;
  values.clear();

  while ( fi.nextFeature( f ) )
  {
    if ( !set.contains( f.attribute( index ).toString() ) )
    {
      values.append( f.attribute( index ) );
      set.insert( f.attribute( index ).toString() );
    }

    if ( limit >= 0 && values.size() >= limit )
      break;
  }
}

QVariant QgsVectorDataProvider::aggregate( QgsAggregateCalculator::Aggregate aggregate, int index,
    const QgsAggregateCalculator::AggregateParameters& parameters, QgsExpressionContext* context, bool& ok ) const
{
  //base implementation does nothing
  Q_UNUSED( aggregate );
  Q_UNUSED( index );
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  ok = false;
  return QVariant();
}

void QgsVectorDataProvider::clearMinMaxCache()
{
  mCacheMinMaxDirty = true;
}

void QgsVectorDataProvider::fillMinMaxCache() const
{
  if ( !mCacheMinMaxDirty )
    return;

  QgsFields flds = fields();
  for ( int i = 0; i < flds.count(); ++i )
  {
    if ( flds.at( i ).type() == QVariant::Int )
    {
      mCacheMinValues[i] = QVariant( INT_MAX );
      mCacheMaxValues[i] = QVariant( INT_MIN );
    }
    else if ( flds.at( i ).type() == QVariant::LongLong )
    {
      mCacheMinValues[i] = QVariant( std::numeric_limits<qlonglong>::max() );
      mCacheMaxValues[i] = QVariant( std::numeric_limits<qlonglong>::min() );
    }
    else if ( flds.at( i ).type() == QVariant::Double )
    {
      mCacheMinValues[i] = QVariant( DBL_MAX );
      mCacheMaxValues[i] = QVariant( -DBL_MAX );
    }
    else
    {
      mCacheMinValues[i] = QVariant();
      mCacheMaxValues[i] = QVariant();
    }
  }

  QgsFeature f;
  QgsAttributeList keys = mCacheMinValues.keys();
  QgsFeatureIterator fi = getFeatures( QgsFeatureRequest().setSubsetOfAttributes( keys )
                                       .setFlags( QgsFeatureRequest::NoGeometry ) );

  while ( fi.nextFeature( f ) )
  {
    QgsAttributes attrs = f.attributes();
    for ( QgsAttributeList::const_iterator it = keys.begin(); it != keys.end(); ++it )
    {
      const QVariant& varValue = attrs.at( *it );

      if ( varValue.isNull() )
        continue;

      if ( flds.at( *it ).type() == QVariant::Int )
      {
        int value = varValue.toInt();
        if ( value < mCacheMinValues[*it].toInt() )
          mCacheMinValues[*it] = value;
        if ( value > mCacheMaxValues[*it].toInt() )
          mCacheMaxValues[*it] = value;
      }
      else if ( flds.at( *it ).type() == QVariant::LongLong )
      {
        qlonglong value = varValue.toLongLong();
        if ( value < mCacheMinValues[*it].toLongLong() )
          mCacheMinValues[*it] = value;
        if ( value > mCacheMaxValues[*it].toLongLong() )
          mCacheMaxValues[*it] = value;
      }
      else if ( flds.at( *it ).type() == QVariant::Double )
      {
        double value = varValue.toDouble();
        if ( value < mCacheMinValues[*it].toDouble() )
          mCacheMinValues[*it] = value;
        if ( value > mCacheMaxValues[*it].toDouble() )
          mCacheMaxValues[*it] = value;
      }
      else
      {
        QString value = varValue.toString();
        if ( mCacheMinValues[*it].isNull() || value < mCacheMinValues[*it].toString() )
        {
          mCacheMinValues[*it] = value;
        }
        if ( mCacheMaxValues[*it].isNull() || value > mCacheMaxValues[*it].toString() )
        {
          mCacheMaxValues[*it] = value;
        }
      }
    }
  }

  mCacheMinMaxDirty = false;
}

QVariant QgsVectorDataProvider::convertValue( QVariant::Type type, const QString& value )
{
  QVariant v( value );

  if ( !v.convert( type ) || value.isNull() )
    v = QVariant( type );

  return v;
}

QgsTransaction* QgsVectorDataProvider::transaction() const
{
  return nullptr;
}

void QgsVectorDataProvider::forceReload()
{
  emit dataChanged();
}

static bool _compareEncodings( const QString& s1, const QString& s2 )
{
  return s1.toLower() < s2.toLower();
}

QStringList QgsVectorDataProvider::availableEncodings()
{
  if ( smEncodings.isEmpty() )
  {
    Q_FOREACH ( const QString& codec, QTextCodec::availableCodecs() )
    {
      smEncodings << codec;
    }
#if 0
    smEncodings << "BIG5";
    smEncodings << "BIG5-HKSCS";
    smEncodings << "EUCJP";
    smEncodings << "EUCKR";
    smEncodings << "GB2312";
    smEncodings << "GBK";
    smEncodings << "GB18030";
    smEncodings << "JIS7";
    smEncodings << "SHIFT-JIS";
    smEncodings << "TSCII";
    smEncodings << "UTF-8";
    smEncodings << "UTF-16";
    smEncodings << "KOI8-R";
    smEncodings << "KOI8-U";
    smEncodings << "ISO8859-1";
    smEncodings << "ISO8859-2";
    smEncodings << "ISO8859-3";
    smEncodings << "ISO8859-4";
    smEncodings << "ISO8859-5";
    smEncodings << "ISO8859-6";
    smEncodings << "ISO8859-7";
    smEncodings << "ISO8859-8";
    smEncodings << "ISO8859-8-I";
    smEncodings << "ISO8859-9";
    smEncodings << "ISO8859-10";
    smEncodings << "ISO8859-11";
    smEncodings << "ISO8859-12";
    smEncodings << "ISO8859-13";
    smEncodings << "ISO8859-14";
    smEncodings << "ISO8859-15";
    smEncodings << "IBM 850";
    smEncodings << "IBM 866";
    smEncodings << "CP874";
    smEncodings << "CP1250";
    smEncodings << "CP1251";
    smEncodings << "CP1252";
    smEncodings << "CP1253";
    smEncodings << "CP1254";
    smEncodings << "CP1255";
    smEncodings << "CP1256";
    smEncodings << "CP1257";
    smEncodings << "CP1258";
    smEncodings << "Apple Roman";
    smEncodings << "TIS-620";
    smEncodings << "System";
#endif
  }

  // Do case-insensitive sorting of encodings
  qSort( smEncodings.begin(), smEncodings.end(), _compareEncodings );

  return smEncodings;
}

void QgsVectorDataProvider::clearErrors()
{
  mErrors.clear();
}

bool QgsVectorDataProvider::hasErrors() const
{
  return !mErrors.isEmpty();
}

QStringList QgsVectorDataProvider::errors() const
{
  return mErrors;
}

bool QgsVectorDataProvider::isSaveAndLoadStyleToDBSupported() const
{
  return false;
}

void QgsVectorDataProvider::pushError( const QString& msg ) const
{
  QgsDebugMsg( msg );
  mErrors << msg;
  emit raiseError( msg );
}

QSet<QgsMapLayerDependency> QgsVectorDataProvider::dependencies() const
{
  return QSet<QgsMapLayerDependency>();
}

QgsGeometry* QgsVectorDataProvider::convertToProviderType( const QgsGeometry& geom ) const
{
  if ( geom.isEmpty() )
  {
    return nullptr;
  }

  QgsAbstractGeometry* geometry = geom.geometry();
  if ( !geometry )
  {
    return nullptr;
  }

  QgsWkbTypes::Type providerGeomType = wkbType();

  //geom is already in the provider geometry type
  if ( geometry->wkbType() == providerGeomType )
  {
    return nullptr;
  }

  QgsAbstractGeometry* outputGeom = nullptr;

  //convert compoundcurve to circularstring (possible if compoundcurve consists of one circular string)
  if ( QgsWkbTypes::flatType( providerGeomType ) == QgsWkbTypes::CircularString && QgsWkbTypes::flatType( geometry->wkbType() ) == QgsWkbTypes::CompoundCurve )
  {
    QgsCompoundCurve* compoundCurve = static_cast<QgsCompoundCurve*>( geometry );
    if ( compoundCurve )
    {
      if ( compoundCurve->nCurves() == 1 )
      {
        const QgsCircularString* circularString = dynamic_cast<const QgsCircularString*>( compoundCurve->curveAt( 0 ) );
        if ( circularString )
        {
          outputGeom = circularString->clone();
        }
      }
    }
  }

  //convert to multitype if necessary
  if ( QgsWkbTypes::isMultiType( providerGeomType ) && !QgsWkbTypes::isMultiType( geometry->wkbType() ) )
  {
    outputGeom = QgsGeometryFactory::geomFromWkbType( providerGeomType );
    QgsGeometryCollection* geomCollection = dynamic_cast<QgsGeometryCollection*>( outputGeom );
    if ( geomCollection )
    {
      geomCollection->addGeometry( geometry->clone() );
    }
  }

  //convert to curved type if necessary
  if ( !QgsWkbTypes::isCurvedType( geometry->wkbType() ) && QgsWkbTypes::isCurvedType( providerGeomType ) )
  {
    QgsAbstractGeometry* curveGeom = outputGeom ? outputGeom->toCurveType() : geometry->toCurveType();
    if ( curveGeom )
    {
      delete outputGeom;
      outputGeom = curveGeom;
    }
  }

  //convert to linear type from curved type
  if ( QgsWkbTypes::isCurvedType( geometry->wkbType() ) && !QgsWkbTypes::isCurvedType( providerGeomType ) )
  {
    QgsAbstractGeometry* segmentizedGeom = nullptr;
    segmentizedGeom = outputGeom ? outputGeom->segmentize() : geometry->segmentize();
    if ( segmentizedGeom )
    {
      delete outputGeom;
      outputGeom = segmentizedGeom;
    }
  }

  //set z/m types
  if ( QgsWkbTypes::hasZ( providerGeomType ) )
  {
    if ( !outputGeom )
    {
      outputGeom = geometry->clone();
    }
    outputGeom->addZValue();
  }
  if ( QgsWkbTypes::hasM( providerGeomType ) )
  {
    if ( !outputGeom )
    {
      outputGeom = geometry->clone();
    }
    outputGeom->addMValue();
  }

  if ( outputGeom )
  {
    return new QgsGeometry( outputGeom );
  }
  return nullptr;
}

void QgsVectorDataProvider::setNativeTypes( const QList<NativeType>& nativeTypes )
{
  mNativeTypes = nativeTypes;
}

QTextCodec* QgsVectorDataProvider::textEncoding() const
{
  return mEncoding;
}

QStringList QgsVectorDataProvider::smEncodings;

QList<QgsRelation> QgsVectorDataProvider::discoverRelations( const QgsVectorLayer*, const QList<QgsVectorLayer*>& ) const
{
  return QList<QgsRelation>();
}
