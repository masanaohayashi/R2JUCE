#include "../modules/R2JUCE/src/R2SequencePlayer.h"
#include <fstream>
#include <iostream>
#include <cstring>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

static void write(const std::string& path,const std::vector<uint8_t>& bytes)
{
    std::ofstream file(path,std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()),std::streamsize(bytes.size()));
    assert(file.good());
}
int main(int argc,char** argv)
{
    assert(argc==2);
    const std::string root=argv[1];
    constexpr size_t header=0x586,trackSize=0x34;
    std::vector<uint8_t> file(header+36*trackSize);
    constexpr char signature[]="RCM-PC98V2.0(C)COME ON MUSIC";
    std::memcpy(file.data(),signature,sizeof(signature)-1);
    file[0x1c0]=48;file[0x1c1]=120;file[0x1c2]=4;file[0x1c3]=4;file[0x1e6]=36;
    for(int track=0;track<36;++track) {
        const size_t base=header+size_t(track)*trackSize;
        file[base]=trackSize;file[base+7]=1;file[base+0x2c]=0xfe;
    }
    const int tracks[]={0,1,17,18,35};
    const int channels[]={0,16,1,2,15};
    const int expectedR36Ports[]={0,1,0,1,1};
    for(int i=0;i<5;++i) {
        const size_t base=header+size_t(tracks[i])*trackSize;
        file[base+4]=uint8_t(channels[i]);file[base+7]=0;
        file[base+0x2c]=uint8_t(60+i);file[base+0x2d]=48;
        file[base+0x2e]=48;file[base+0x2f]=100;file[base+0x30]=0xfe;
    }
    const std::string r36=root+"/player-dual.R36",rcp=root+"/player-dual.rcp";
    write(r36,file);write(rcp,file);
    for(const auto& path:{r36,rcp}) {
        r2juce::SequenceFile song;std::string error;
        if(!song.load(path,error,false)) {std::cerr<<error<<std::endl;return 1;}
        assert(song.events.size()==10);
        for(const auto& e:song.events) {
            const int i=int(e.bytes[1])-60;
            assert(i>=0 && i<5);
            assert((e.bytes[0]&15)==(channels[i]&15));
            assert(e.port==(path==r36?expectedR36Ports[i]:(channels[i]>>4)));
            assert(std::abs(e.seconds-((e.bytes[0]&0xf0)==0x90?0.0:0.5))<1e-9);
        }
        assert(std::abs(song.totalSeconds()-2.5)<1e-9);
    }
    r2juce::SequencePlayer player;std::string error;
    r2juce::SequenceFile loaded; assert(loaded.load(r36,error,false)); player.load(&loaded);
    std::array<std::vector<uint8_t>,2> sent;
    auto send=[&](const uint8_t* b,int size,int,int port){assert(port>=0 && port<2);sent[size_t(port)].insert(sent[size_t(port)].end(),b,b+size);};
    player.processStamped(0,48000,send,[]{});sent[0].clear();sent[1].clear();
    player.play();player.processStamped(1,48000,send,[]{});
    assert((sent[0]==std::vector<uint8_t>{0x90,60,100,0x91,62,100}));
    assert((sent[1]==std::vector<uint8_t>{0x90,61,100,0x92,63,100,0x9f,64,100}));
    for(bool stop:{false,true}) {
        sent[0].clear();sent[1].clear();
        if(stop) player.stop();else player.pause();
        player.processStamped(0,48000,send,[]{});
        for(int port=0;port<2;++port) {
            assert(sent[size_t(port)].size()==size_t(16*(stop?15:12)));
            for(int ch=0;ch<16;++ch) for(int cc:{64,66,123,120,121}) {
                if(cc==121 && !stop) continue;
                bool found=false;
                for(size_t i=0;i<sent[size_t(port)].size();i+=3)
                    if(sent[size_t(port)][i]==uint8_t(0xb0|ch) && sent[size_t(port)][i+1]==cc) found=true;
                assert(found);
            }
        }
    }
    // Replacing a B song with A-only still silences the retired B destination.
    r2juce::SequenceFile aOnly;
    aOnly.events.push_back({0.0,{0x90,67,100},0});
    player.load(&aOnly);sent[0].clear();sent[1].clear();
    player.processStamped(0,48000,send,[]{});
    assert(sent[1].size()==240);
    // SMF port metadata applies to the following events (including SysEx).
    const std::vector<uint8_t> track{0,0xff,0x21,1,1,0,0x90,60,100,
        0,0xf0,2,0x7e,0xf7,0,0xff,0x21,1,0,0,0x80,60,0,0,0xff,0x2f,0};
    std::vector<uint8_t> smf{'M','T','h','d',0,0,0,6,0,0,0,1,0,48,
        'M','T','r','k',0,0,0,uint8_t(track.size())};
    smf.insert(smf.end(),track.begin(),track.end());write(root+"/ports.mid",smf);
    r2juce::SequenceFile midi;assert(midi.load(root+"/ports.mid",error,false));
    assert(midi.events.size()==3 && midi.events[0].port==1 && midi.events[1].port==1 && midi.events[2].port==0);
    // A fixture for the real processor: a held note on B1, with NO A events.
    auto bOnly=file;
    for(int i=0;i<36;++i) bOnly[header+size_t(i)*trackSize+7]=i==1?0:1;
    bOnly[header+trackSize+0x2d]=127;bOnly[header+trackSize+0x2e]=127;
    write(root+"/player-b-only.r36",bOnly);
    file.resize(32);write(root+"/broken.r36",file);
    r2juce::SequenceFile broken; assert(!broken.load(root+"/broken.r36",error,false));
    assert(player.hasSequence());
    // Existing callback signatures must still compile and receive messages.
    int legacyMessages=0;
    player.stop();
    player.processStamped(0,48000,[&](const uint8_t*,int,int){++legacyMessages;},[]{});
    assert(legacyMessages==80);
    player.play();
    player.process(1,48000,[](const uint8_t*,int){},[](int,int){},[](int){return 1;});
    player.stop();
    player.process(0,48000,[](const uint8_t*,int,int){},[](int,int){},[](int){return 0;});
    std::cout<<"PASS: R36 case-insensitive loading, explicit B channels, 18/19-track boundary, RCP compatibility, A/B scheduling and panic, malformed input"<<std::endl;
}
