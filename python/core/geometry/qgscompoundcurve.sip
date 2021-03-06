class QgsCompoundCurve: public QgsCurve
{
%TypeHeaderCode
#include <qgscompoundcurve.h>
%End

  public:
    QgsCompoundCurve();
    QgsCompoundCurve( const QgsCompoundCurve& curve );
    //QgsCompoundCurve &operator=( const QgsCompoundCurve & curve );
    ~QgsCompoundCurve();

    virtual bool operator==( const QgsCurve& other ) const;
    virtual bool operator!=( const QgsCurve& other ) const;

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsCompoundCurve* clone() const /Factory/;
    virtual void clear();

    virtual bool fromWkb( QgsConstWkbPtr wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual void points( QList<QgsPointV2>& pts ) const;
    virtual int numPoints() const;
    /** Returns a new line string geometry corresponding to a segmentized approximation
     * of the curve.
     * @param tolerance segmentation tolerance
     * @param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    virtual QgsLineString* curveToLine( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

    /** Returns the number of curves in the geometry.
     */
    int nCurves() const;

    /** Returns the curve at the specified index.
     */
    const QgsCurve* curveAt( int i ) const;

    /** Adds a curve to the geometr (takes ownership)
     */
    void addCurve( QgsCurve* c /Transfer/ );

    /** Removes a curve from the geometry.
     * @param i index of curve to remove
     */
    void removeCurve( int i );

    /** Adds a vertex to the end of the geometry.
     */
    void addVertex( const QgsPointV2& pt );

    void draw( QPainter& p ) const;
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                    bool transformZ = false );
    void transform( const QTransform& t );
    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

    virtual bool insertVertex( QgsVertexId position, const QgsPointV2& vertex );
    virtual bool moveVertex( QgsVertexId position, const QgsPointV2& newPos );
    virtual bool deleteVertex( QgsVertexId position );

    virtual double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const;

    void sumUpArea( double& sum ) const;

    /** Appends first point if not already closed.*/
    void close();

    bool hasCurvedSegments() const;

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
        @param vertex the vertex id
        @return rotation in radians, clockwise from north*/
    double vertexAngle( QgsVertexId vertex ) const;

    virtual QgsCompoundCurve* reversed() const /Factory/;

    virtual bool addZValue( double zValue = 0 );
    virtual bool addMValue( double mValue = 0 );

    virtual bool dropZValue();
    virtual bool dropMValue();

    virtual double xAt( int index ) const;
    virtual double yAt( int index ) const;

  protected:

    virtual QgsRectangle calculateBoundingBox() const;

};
