# SynthFM

## Предварительные требования

- Windows 10 или новее.
- [Visual Studio 2019 или более поздняя версия](https://visualstudio.microsoft.com/ru/) с установленными инструментами разработки на C++.
- [Фреймворк JUCE](https://juce.com/)

## Клонирование репозитория

Склонируйте репозиторий

```bash
git clone https://github.com/ainthero/SynthFM
```

## Сборка
Для сборки плагина выполните следующие шаги:

1. Откройте файл `SynthFM.jucer` с помощью Projucer (часть фреймворка JUCE).
2. Нажмите кнопку "Save and Open in IDE" для генерации файлов проекта Visual Studio.
3. Откройте сгенерированный файл `.sln` в Visual Studio.
4. В обозревателе решений выберите какую версию плагина собрать - standalone или vst3.
5. Нажмите Build.

## Запуск
* Standalone версия не требует дополнительных действий - это отдельное приложение. По умолчанию собирается в `SynthFM\Builds\VisualStudio2022\x64\Release\Standalone Plugin\SynthFM.exe`
* VST3 файл необходимо скопировать из `SynthFM\Builds\VisualStudio2022\x64\Release\VST3\SynthFM.vst3\Contents\x86_64-win\SynthFM.vst3` в `C:\Program Files\Common Files\VST3`. Плагин станет доступен в DAW.
