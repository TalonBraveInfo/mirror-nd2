--all methods are public
--private member (generated by soundMgr): object
--private member (generated by soundMgr): name
SoundT = {
    volume = 1,
    maxVolume = 1,
    
    Play = function( self )
        self.object:stop()
        self.object:play()
    end,
    
    Stop = function( self )
        self.object:stop()
    end,
    
    SetFadeParams = function( self, fadeInRate, fadeOutRate )
        self.fadeInRate = fadeInRate
        self.fadeOutRate = fadeOutRate
    end,
    
    SetMaxVolume = function( self, maxVolume )
        if maxVolume > 1 then maxVolume = 1
        elseif maxVolume < 0 then maxVolume = 0 
        end
        self.maxVolume = maxVolume
        if self.volume > self.maxVolume then
           self:SetVolume( self.maxVolume )
        end
    end,
    
    SetVolume = function( self, volume )
        if volume > 1 then volume = 1
        elseif volume < 0 then volume = 0 
        end
        self.object:setvolume( volume )
        self.volume = volume
    end,
    
    GetVolume = function( self )
        return self.volume
    end,
    
    FadeIn = function( self )
        assert( self.fadeInRate ~= nil )
        self.fading = true
        soundMgr:FadeIn( self )
    end,
    
    FadeOut = function( self )
        assert( self.fadeOutRate ~= nil )
        self.fading = true
        soundMgr:FadeOut( self )
    end,
    
    SetPosition = function( self, x, y, z )
        self.object:setposition(x,y,z)
    end
}

soundMgr = {   
    fadeInSounds = {},
    fadeOutSounds = {},
    allSounds = {}, -- this could be divided into groups, so that we can (e.g.) increase or decrease the volume of a subset of sounds
    massFadeInRate = 0.04,
    massFadeOutRate = 0.04,
    
    CreateSound = function( self, filename, loop, ambient, initialVolume )
        self.fadeInSounds[filename] = nil 
        self.fadeOutSounds[filename] = nil 
        selSafe( 'ccsound', filename )
            local retVal = Instantiate( SoundT, { name = filename, object = lookup(getCwd()) } ) 
            retVal.object:init( 'sounds:'..filename..'.wav', loop, ambient )
            if initialVolume ~= nil and initialVolume < 1 then -- 1 is the default
                retVal:SetVolume( initialVolume )
            end
        sel('..')
        self.allSounds[ filename ] = retVal
        return retVal
    end,
    
    SetMasterVolume = function( self, volume )
        assert( volume >= 0 and volume <= 1 )
        nebula.sys.servers.audio:setmastervolume( volume )
    end,
    
    Trigger = function( deltaTime )
        self = soundMgr -- HACK!
        self:DoFades()
    end,
    
    MassFadeIn = function( self )
        self.massFade = 'in'
        self.massVolume = 0
    end,
    
    MassFadeOut = function( self )
        self.massFade = 'out'
        self.massVolume = 1
    end,
    
    FadeIn = function( self, sound )
        self.fadeInSounds[sound.name] = sound
        self.fadeOutSounds[sound.name] = nil
    end,
    
    FadeOut = function( self, sound )
        self.fadeInSounds[sound.name] = nil
        self.fadeOutSounds[sound.name] = sound
    end,
    
    DoFades = function( self, deltaTime )
        -- fade sounds in or out
        if self.massFade == nil then
            local removeList = {}
            for idx, sound in self.fadeOutSounds do
                local volume = sound.volume - sound.fadeOutRate
                if volume <= 0 then
                    volume = 0
                    sound:Stop()
                    table.insert( removeList, sound.name )
                    sound.fading = false
                end
                sound:SetVolume( volume )
            end
            for idx, soundName in removeList do
                self.fadeOutSounds[ soundName ] = nil
            end
            
            removeList = {}
            for idx, sound in self.fadeInSounds do
                local volume = sound.volume
                if volume <= 0 then
                    sound:Play()
                    volume = 0
                end
                volume = volume + sound.fadeInRate
                if volume >= sound.maxVolume then
                    table.insert( removeList, sound.name )
                    sound.fading = false
                    volume = sound.maxVolume
                end
                sound:SetVolume( volume )
            end
            for idx, soundName in removeList do
                self.fadeInSounds[ soundName ] = nil
            end
        else
        --we neither play nor stop anything, since we only want to affect those sounds that 
        --are already playing.
            if self.massFade == 'in' then
                self.massVolume = self.massVolume + self.massFadeInRate
                if self.massVolume >= 1 then
                    self.massVolume = 1
                    self.massFade = nil
                end
            else -- == 'out'
                self.massVolume = self.massVolume - self.massFadeOutRate
                if self.massVolume <= 0 then
                    self.massVolume = 0
                    self.massFade = nil
                end
            end
            for name, sound in self.allSounds do
                sound.object:setvolume( sound.volume * self.massVolume ) -- we don't use self:SetVolume because we don't want to change the sound's original volume
            end
        end
    end
}

