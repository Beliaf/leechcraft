#ifndef PLUGINS_LMP_CORE_H
#define PLUGINS_LMP_CORE_H
#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/structures.h>
#include "phonon.h"
#include "player.h"

namespace Phonon
{
	class VideoWidget;
	class SeekSlider;
	class VolumeSlider;
};

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			class Core : public QObject
			{
				Q_OBJECT

				std::auto_ptr<Phonon::MediaObject> MediaObject_;
				std::auto_ptr<Phonon::AudioOutput> AudioOutput_;
				bool TotalTimeAvailable_;
				Phonon::VideoWidget *VideoWidget_;
				Phonon::SeekSlider *SeekSlider_;
				Phonon::VolumeSlider *VolumeSlider_;
				Phonon::Path VideoPath_;
				Phonon::Path AudioPath_;
				std::auto_ptr<Player> Player_;
				ICoreProxy_ptr Proxy_;
				QAction *ShowAction_;

				Core ();
			public:
				enum SkipAmount
				{
					SkipLittle = 10
					, SkipMedium = 60
					, SkipALot = 600
				};

				static Core& Instance ();
				void Release ();
				void SetCoreProxy (ICoreProxy_ptr);
				ICoreProxy_ptr GetCoreProxy () const;

				void Reinitialize ();
				Phonon::MediaObject* GetMediaObject () const;
				void SetVideoWidget (Phonon::VideoWidget*);
				void SetSeekSlider (Phonon::SeekSlider*);
				void SetVolumeSlider (Phonon::VolumeSlider*);
				void IncrementVolume ();
				void DecrementVolume ();
				void ToggleFullScreen ();
				void TogglePause ();
				void Forward (SkipAmount);
				void Rewind (SkipAmount);
				QAction* GetShowAction () const;
				void Handle (const LeechCraft::DownloadEntity&);
			public slots:
				void play ();
				void pause ();
				void setSource (const QString&);
			private slots:
				void updateState ();
				void totalTimeChanged ();
				void handleHasVideoChanged (bool);
			signals:
				void stateUpdated (const QString&);
				void error (const QString&);
				void bringToFront ();
			};
		};
	};
};

#endif

