import sys, pathlib, urllib.request, concurrent.futures, json
sys.path.insert(0,'build/audio-tools')
import soundfile as sf
import numpy as np, hashlib
names=['REIMU','ST0','POSITIVE','ST1','ST2','LEGEND','ST3','ST4','KAMI','ST5','TENSI','SYUGEN','ALICE','IRIS','ZIPANGU','KAMI2']
hashes='zriqhmpo ycqxobre xzeamhcn jiliirkx hsurdrvg suwzisor ripzdvpo zgfklyjy qitvwtgz ezjvbvan mzkawhxq kfvofjet fwtdijuw xxxuepki hmolkuqy rqfbqbim'.split()
titles=['A Sacred Lot','Eternal Shrine Maiden','The Positive and Negative','Highly Responsive to Prayers','Eastern Tale of Fancy',"Angel\'s Legend",'Oriental Magician','Small Evil-Crushing Blade','Magic Mirror','The Legend of Kage','Now, Until the Moment You Die','We Shall Die Together','Swordsman of a Distant Star','Iris','Shrine of the Wind','Magic Mirror (Makai Route)']
from urllib.parse import quote
out=pathlib.Path('build/recorded-bgm');(out/'source').mkdir(parents=True,exist_ok=True)
def run(i):
 url='https://nu.vgmtreasurechest.com/soundtracks/touhou-reiiden-highly-responsive-to-prayers.-pc-9801/'+hashes[i]+'/'+quote(f'{i+1:02}. {titles[i]}')+'.flac'
 p=out/'source'/(names[i]+'.flac')
 if not p.exists():
  req=urllib.request.Request(url,headers={'User-Agent':'Mozilla/5.0'})
  p.write_bytes(urllib.request.urlopen(req,timeout=90).read())
 x,r=sf.read(p,dtype='int16',always_2d=True);sf.write(out/(names[i]+'.wav'),x,r,subtype='PCM_16')
 y,rr=sf.read(out/(names[i]+'.wav'),dtype='int16',always_2d=True)
 assert r==rr and np.array_equal(x,y), 'PCM conversion mismatch'
 print(names[i],r,len(x),flush=True)
 return dict(stem=names[i],url=url,frames=len(x),rate=r,source_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),wav_sha256=hashlib.sha256((out/(names[i]+'.wav')).read_bytes()).hexdigest())
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool: result=list(pool.map(run,range(16)))
(out/'downloads.json').write_text(json.dumps(result,indent=2))
