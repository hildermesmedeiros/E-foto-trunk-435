#ifndef SCIENCESPINBOX
#define SCIENCESPINBOX

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QLineEdit>
#include <QtGui/QMouseEvent>
#include <QtCore/QVariant>
#include <QtCore/QDebug>
#include <QtCore/QString>

// This class contains code of Matthias Pospiech to reimplement some of the validation methods.

static bool isIntermediateValueHelper(qint64 num, qint64 minimum, qint64 maximum, qint64 *match = 0);

class ScienceSpinBox : public QDoubleSpinBox
{
Q_OBJECT
public:
	ScienceSpinBox(QWidget * parent = 0);

	int decimals() const;
	void setDecimals(int value);

	QString textFromValue ( double value ) const;
	double valueFromText ( const QString & text ) const;

	void setFullRange();

	QString mantissa(bool forceDot = false) const;
	QString exponent() const;
	QString textValue() const;
	void setTextValue(QString value);
	void selectMantissa();
	void selectExponent();

private:
	int dispDecimals;
	QChar delimiter, thousand;
	QDoubleValidator * v;


private:
	void initLocalValues(QWidget *parent);
	int delimiterPosition() const;
	int exponentialPosition() const;
	bool eventFilter(QObject *obj, QEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void focusInEvent(QFocusEvent *event);
	void timerEvent(QTimerEvent *event);
	bool isIntermediateValue(const QString &str) const;
	QVariant validateAndInterpret(QString &input, int &pos, QValidator::State &state) const;
	QValidator::State validate(QString &text, int &pos) const;
	void fixup(QString &input) const;
	QString stripped(const QString &t, int *pos) const;
	double round(double value) const;
	void stepBy(int steps);

public slots:
	void stepDown();
	void stepUp();
	void stepDownMantissa();
	void stepUpMantissa();

};

#endif //SCIENCESPINBOX
