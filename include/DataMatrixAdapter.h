class DataMatrixAdapter
{
public:
  typedef float value_type;
  typedef float coord_type;

  typedef NFmiDataMatrix<float>::size_type size_type;

  DataMatrixAdapter(const NFmiDataMatrix<float> & theMatrix)
        : itsMatrix(theMatrix)
  { }

  const value_type & operator()(size_type i, size_type j) const
  {
        return itsMatrix[i][j];
  }

  float x(size_type i, size_type j) const { return static_cast<float>(i); }
  float y(size_type i, size_type j) const { return static_cast<float>(j); }

  size_type width()  const { return itsMatrix.NX(); }
  size_type height() const { return itsMatrix.NY(); }


private:
  DataMatrixAdapter();
  const NFmiDataMatrix<float> & itsMatrix;

}; // class DataMatrixAdapter


