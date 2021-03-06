#ifndef XMLUPDATER_H
#define XMLUPDATER_H
/*Copyright 2002-2014 e-foto team (UERJ)
  This file is part of e-foto.

    e-foto is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    e-foto is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with e-foto.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "EDom.h"
//#include <string>
//#include <deque>

namespace br {
namespace uerj {
namespace eng {
namespace efoto {

/**
* \file XmlUpdater.h
* \class XmlUpdater
* \brief Classe para garantir a conversão de arquivos (.epp) de versões anteriores para a versão corrente.
* \copyright E-Foto group
* \authors Rafael Aguiar, Irving Badolato & Paulo Pupim
*/
class XmlUpdater
{

private:
    std::deque <std::string> builds;
    std::string xmlBuild; // this is a version descriptor of loaded xml
    std::string referenceBuild; // this is a version descriptor of reference
	EDomElement allXml; // this is a xml loaded under constructor

	int error;
	bool updated;

	/**
 * \brief Método para comparar duas versões.
 * \param buildOne	Versão final. Normalmente a versão de referência.
 * \param buidTwo	Versão inicial. Normalmente a versão que se deseja atualizar.
 * \return int	Valor 1 se a atualização é possível, 0 se não é necessária, -1 é impossível, pois equivale a voltar uma versão, -2 é impossível, pois a buildOne é desconhecida ou -3 se é impossível, pois buildTwo é desconhecida.
 */
    int compareBuilds(std::string buildOne, std::string buildTwo);

	/**
 * \brief Método interno que executa as chamadas de update necessárias em cascata.
 */
	void executeUpdate();

public:

	/**
 * \brief Construtor padrão para receber um xml proveniente de arquivo epp e atualizá-lo imediatamente.
 * \param allXml	Todo o conteúdo de um arquivo epp.
 * \param referenceBuild	Versão de referência para qual o xml deve ser atualizado. Por padrão é a última versão conhecida pela classe.
 */
    explicit XmlUpdater(std::string allXml, std::string referenceBuild = "");

	/**
 * \brief Método para retornar o conteúdo do arquivo xml armazenado.
 * \return EDomElement	Elemento contendo o xml (epp) atualizado.
 */
	EDomElement getAllXml();

	/**
 * \brief Método para retornar a avaliação do processo de atualização.
 * \return bool	Retorna verdadeiro se houve erro na atualização e falso em caso contrário.
 */
	int getError();

	/**
 * \brief Método que verifica se o arquivo epp está atualizado.
 * \return bool	Retorna verdadeiro se o arquivo epp está atualizado e falso em caso contrário.
 */
	bool isUpdated();

	/**
 * \brief Método para retornar a versão do arquivo epp.
 * \return string	Retorna a descritor de versão registrado no arquivo epp.
 */
    std::string getXmlBuild();

	/**
 * \brief Método para retornar a versão mais atual de arquivo epp conhecida por esta classe.
 * \return string	Retorna a descritor da última versão reconhecida por esta classe.
 */
    std::string getReferenceBuild();

	/**
 * \brief Método para informar se uma versão de arquivo epp é uma versão conhecida.
 * \param build	Descritor da versão a ser avaliada.
 * \return string	Retorna verdadeiro se a versão é reconhecida pela classe.
 */
    bool buildIsValid(std::string build);

	/**
 * \brief Método para atualizar o arquivo epp é até a última versão conhecida.
 * \param error	variável para armazenar o código do erro se ele existir. Se é impossível atualizar o xml ele retorna 1 se o processo equivale a voltar versões, 2 se a versão de destino é desconhecida, 3 se a versão do arquivo lido é desconhecida ou 4 se o arquivo está vazio.
 * \return string	Retorna verdadeiro se a versão é reconhecida pela classe.
 */
	bool updateBuild(int *error);

	/**
 * \brief Método para aplicar a atualização para a versão 1.0.42 do arquivo .epp.
 */
	void updateToBuild1_0_42();

	/**
 * \brief Método para aplicar a atualização para a versão 1.0.266 do arquivo .epp.
 */
	void updateToBuild1_0_266();

	/**
 * \brief Método para aplicar a atualização para a versão 1.0.325 do arquivo .epp.
 */
	void updateToBuild1_0_325();

	/**
	  * \brief Metodo para substituir uma palavra por outra no texto
	  * \param text: Text contendo a palavra a se substituida
	  * \param oldWord: Palavra a ser substituida
	  * \param newWord: Palavra a ser colocada no lugar da palavra procurada
	  * \return bool : Informa se a palavra foi encontrada ou nao, no texto.
	  */
    bool replacer(std::string &text,std::string oldWord,std::string newWord);

};

} // namespace efoto
} // namespace eng
} // namespace uerj
} // namespace br

#endif // XMLUPDATER_H
